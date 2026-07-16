module;

#include <string>
#include <format>

#include <asio/ip/tcp.hpp>
#include <asio/connect.hpp>
#include <nlohmann/json.hpp>

export module BEP20;

import logger_pc;
import serial;
import transaction;
import commands;
import result;

using json = nlohmann::json;

constexpr double WEI_IN_TOKEN = 1'000'000'000'000'000'000.0;

export enum class SupportedTokens{
    BNB, USDC
};

typedef struct {
    std::string_view host;
    uint8_t chainID;
} BSCMainnet;

constexpr BSCMainnet bsc_mainnet{
    .host = "bsc-dataseed.bnbchain.org",
    .chainID = 0x38
};

export typedef struct {
    double bnb;
    double usdc;
} Balance;

export class BEP20 : public Serial {
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver resolver;
    std::string address;

    json post_request(json const& request) {
        auto const endpoint = resolver.resolve(bsc_mainnet.host, "80");
        asio::connect(socket, endpoint);

        std::string final_request = std::format(
                "POST / HTTP/1.1\r\n"
                "Host: {}\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: {}\r\n"
                "Connection: close\r\n\r\n"
                "{}",
                bsc_mainnet.host, request.dump().length(), request.dump()
            );

        std::string response;
        char buffer[512];
        std::error_code ec;

        socket.write_some(asio::buffer(final_request));

        while (response.find("\r\n\r\n") == std::string::npos) {
            size_t n = socket.read_some(asio::buffer(buffer), ec);
            if (ec) throw std::system_error(ec);
            response.append(buffer, n);
        }

        size_t content_length = 0;
        size_t pos = response.find("Content-Length: ");
        if (pos != std::string::npos) {
            pos += 16;
            size_t const end = response.find("\r\n", pos);
            content_length = std::stoul(response.substr(pos, end - pos));
        }

        size_t const header_end = response.find("\r\n\r\n") + 4;
        std::string body = response.substr(header_end);

        while (body.length() < content_length) {
            size_t n = socket.read_some(asio::buffer(buffer), ec);
            if (ec == asio::error::eof) break;
            if (ec) throw std::system_error(ec);
            body.append(buffer, n);
        }

        return json::parse(body);
    }

    double hex_string_to_double(const std::string& hex) {
        double result = 0.0;

        for (char const c : hex) {
            result *= 16.0;

            if (c >= '0' && c <= '9') {
                result += (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                result += (c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                result += (c - 'A' + 10);
            }
        }

        return result;
    }

    double response_to_double(const json& response) {
        if (response.contains("error")) {
            Logger::instance().write(
                std::format("JSON-RPC error: {}",
                    response["error"]["message"].get<std::string>()),
                LogType::Error
            );
            return 0.0;
        }

        std::string hex_balance = response["result"].get<std::string>();

        if (hex_balance.starts_with("0x") || hex_balance.starts_with("0X")) {
            hex_balance = hex_balance.substr(2);
        }

        if (hex_balance.empty()) return 0.0;

        size_t const first_non_zero = hex_balance.find_first_not_of('0');
        if (first_non_zero == std::string::npos) return 0.0;
        hex_balance = hex_balance.substr(first_non_zero);

        double const value = hex_string_to_double(hex_balance);

        return value / WEI_IN_TOKEN;
    }

    uint64_t eth_getTransactionCount() {
        json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "eth_getTransactionCount";
        request["params"] = json::array({address, "pending"});
        request["id"] = 1;

        json response = post_request(request);
        return std::stoull(response["result"].get<std::string>(), nullptr, 16);
    }

    std::array<uint64_t, 2> eth_gasPrice() {
        json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "eth_gasPrice";
        request["params"] = json::array();
        request["id"] = 1;

        json response = post_request(request);

        return response["result"].get<std::array<uint64_t, 2>>();
    }

    uint64_t eth_estimateGas(
        std::array<uint8_t, 20> const& to_address,
        std::optional<std::array<uint8_t, 178>> const& data,
        std::string const& wei_amount
    ) {
        json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "eth_estimateGas";
        request["params"] = json::array({json::object({
            {"from", address},
            {"to", to_address},
            {"value", wei_amount}
        })});
        request["id"] = 1;

        if (data.has_value()) {
            std::string data_hex = "0x";
            for (uint8_t byte : data.value()) {
                data_hex += std::format("{:02x}", byte);
            }
            request["params"][0]["data"] = data_hex;
        }

        json response = post_request(request);
        return response["result"].get<uint64_t>();
    }

    Result eth_sendRawTransaction(std::vector<char> hash) {
        json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "eth_sendRawTransaction";
        request["params"] = json::array({hash});

        json response = post_request(request);
        return response.contains("result") ? Result::Ok : Result::Err;
    }
public:
    explicit BEP20(asio::io_context& context, std::string const& port_name) :
        Serial(context, port_name),
        socket(context),
        resolver(context)
    {
        std::optional res = get_address_from_mcu();
        if (!res.has_value())
            throw std::runtime_error("error receiving addr from mcu");

        address = res.value();
    }

    [[nodiscard]] Balance get_balance() {
        json const response_bnb = post_request(json{
            {"jsonrpc", "2.0"},
            {"method", "eth_getBalance"},
            {"params", {address, "latest"}},
            {"id", 1}
        });

        constexpr std::string_view USDC_CONTRACT =
            "0x8ac76a51cc950d9822d68b83fe1ad97b32cd580d";

        std::string tmp_address = address;
        if (tmp_address.starts_with("0x")) {
            tmp_address = tmp_address.substr(2);
        }

        std::string data = "0x70a08231" + std::string(24, '0') + tmp_address;

        json request_usdc;
        request_usdc["jsonrpc"] = "2.0";
        request_usdc["method"] = "eth_call";
        request_usdc["params"] = json::array({
            json::object({
                {"to", USDC_CONTRACT},
                {"data", data}
            }),
            "latest"
        });
        request_usdc["id"] = 1;

        json const response_usdc = post_request(request_usdc);

        return Balance{
            .bnb = response_to_double(response_bnb),
            .usdc = response_to_double(response_usdc)
        };
    }

    std::string get_address_cached() {
        return address;
    }

    void send_transaction(
        std::string const& to_addr,
        SupportedTokens const token,
        double const amount
    ) {
        uint64_t const wei = static_cast<uint64_t>(amount * WEI_IN_TOKEN);
        std::string const wei_amount = std::format("{:#x}", wei);

        uint64_t nonce = eth_getTransactionCount();
        std::array<uint64_t, 2> gas_price = eth_gasPrice();

        std::array<uint8_t, 20> to_address{};
        std::optional<std::array<uint8_t, 178>> data = std::nullopt;
        std::array<uint64_t, 2> value = {0, 0};

        if (token == SupportedTokens::BNB) {
            std::string clean_to = to_addr;
            if (clean_to.starts_with("0x")) {
                clean_to = clean_to.substr(2);
            }
            for (size_t i = 0; i < 20; ++i) {
                to_address[i] = static_cast<uint8_t>(
                    std::stoi(clean_to.substr(i * 2, 2), nullptr, 16)
                );
            }
        } else if (token == SupportedTokens::USDC) {
            std::string contract = "8ac76a51cc950d9822d68b83fe1ad97b32cd580d";
            for (size_t i = 0; i < 20; ++i) {
                to_address[i] = static_cast<uint8_t>(
                    std::stoi(contract.substr(i * 2, 2), nullptr, 16)
                );
            }

            constexpr std::string_view signature = "a9059cbb";

            std::string clean_to = to_addr;
            if (clean_to.starts_with("0x")) {
                clean_to = clean_to.substr(2);
            }

            std::string data_str(signature);
            data_str += std::string(64 - clean_to.length(), '0') + clean_to;
            data_str += std::string(64 - (wei_amount.length() - 2), '0') + wei_amount.substr(2);

            std::array<uint8_t, 178> data_bytes{};
            for (size_t i = 0; i < data_str.length() / 2 && i < 178; ++i) {
                data_bytes[i] = static_cast<uint8_t>(
                    std::stoi(data_str.substr(i * 2, 2), nullptr, 16)
                );
            }
            data = data_bytes;
        }

        uint64_t const estimated_gas = eth_estimateGas(to_address, data, wei_amount);

        Transaction transaction(
            nonce,
            gas_price,
            estimated_gas,
            to_address,
            value,
            data
        );

        auto transaction_arr = transaction.to_bytes();
        std::visit([this] (const auto& tr) {
            if (send_command(command_sign_transaction) == Result::Err)
                throw std::runtime_error("the signing transaction failed while sending the command");

            Result const res = send(std::span<const char>(reinterpret_cast<const char*>(tr.data()), tr.size()));
            if (res == Result::Err)
                throw std::runtime_error("the signing transaction failed while sending data");

            std::optional<std::vector<char>> const resp = receive();
            if (!resp.has_value())
                throw std::runtime_error("the signing transaction failed while receiving data");

            std::vector<char> const& hash = resp.value();

            eth_sendRawTransaction(hash);
        }, transaction_arr);
    }
};