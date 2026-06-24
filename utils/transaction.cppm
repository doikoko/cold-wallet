module;

#include <optional>
#include <string>
#include <sstream>
#include <cstring>

#include <nlohmann/json.hpp>

export module transaction;

export class Transaction {
    unsigned int nonce;
    std::string_view gas_price;
    std::string_view gas_limit;
    std::string_view to;
    std::string_view value;
    std::optional<std::string_view> data;

    std::optional<std::string_view> parse_dyn(char const*& ptr) {
        uint8_t const len = *ptr++;
        if (len == 0) return std::nullopt;
        std::string_view const view(ptr, len);
        ptr += len;
        return view;
    }
public:
    Transaction(
        unsigned char const nonce,
        std::string_view const gas_price,
        std::string_view const gas_limit,
        std::string_view const to,
        std::string_view const value,

        std::optional<std::string_view> const& data
    ) : nonce(nonce),
        gas_price(gas_price),
        gas_limit(gas_limit),
        to(to),
        value(value),
        data(data){}

    Transaction(std::string_view const str) {
        char const* ptr = str.data();
        std::memcpy(&nonce, ptr, 4);
        ptr += 4;

        gas_price = parse_dyn(ptr).value();
        gas_limit = parse_dyn(ptr).value();
        to = parse_dyn(ptr).value();
        value = parse_dyn(ptr).value();
        data = parse_dyn(ptr).value();
    }

    nlohmann::json to_json() {
        return nlohmann::json{
            {"nonce", nonce},
            {"gas_price", gas_price},
            {"gas_limit", gas_limit},
            {"to", to},
            {"value", value}
            {"data", data.value_or("")}
        };
    }

    std::string&& to_str() const {
        std::stringstream ss;

        ss << nonce
            << gas_price.size() << gas_price
            << gas_limit.size() << gas_limit
            << to.size() << to
            << value.size() << value
            << data.value_or("").size() << data.value_or("");

        return ss.str();
    }
};

