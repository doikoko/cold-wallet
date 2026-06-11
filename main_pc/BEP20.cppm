module;

#include <string>
#include <print>

#include <asio/ip/tcp.hpp>
#include <asio/connect.hpp>
#include <nlohmann/json.hpp>

#include <TWAnySigner.h>

export module BEP20;

import logger_pc;

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

export class BEP20 {
    asio::ip::tcp::socket socket;
    asio::ip::tcp::resolver resolver;
    std::string address;

    json get_request(json const& request) {
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
            pos += 16; // "Content-Length: " length
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
public:
    explicit BEP20(asio::io_context& context, std::string address) :
        socket(context),
        resolver(context),
        address(std::move(address)) {}

    [[nodiscard]] Balance get_balance() {
        json const response_bnb = get_request(json{
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

        json const response_usdc = get_request(request_usdc);

        return Balance{
            .bnb = response_to_double(response_bnb),
            .usdc = response_to_double(response_usdc)
        };
    }

    std::string get_address_cached() {
        return address;
    }

    void send_transaction(){}
};