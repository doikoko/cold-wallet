module;

#include <optional>
#include <variant>
#include <cstdint>

#include <nlohmann/json.hpp>

export module transaction;

export class Transaction {
    uint64_t const nonce;
    std::array<uint64_t, 2> const gas_price;
    uint64_t const gas_limit;
    std::array<uint8_t, 20> const to;
    std::array<uint64_t, 2> const value;
    std::optional<std::array<uint8_t, 178>> const data;
public:
    Transaction(
        uint64_t const& nonce,
        std::array<uint64_t, 2> const& gas_price,
        uint64_t const& gas_limit,
        std::array<uint8_t, 20> const& to,
        std::array<uint64_t, 2> const& value,
        std::optional<std::array<uint8_t, 178>> const& data
    ) : nonce(nonce),
        gas_price(gas_price),
        gas_limit(gas_limit),
        to(to),
        value(value),
        data(data){}

    std::variant<std::array<uint8_t, 38>, std::array<uint8_t, 216>> to_bytes() const {
        if (data.has_value()) {
            std::array<uint8_t, 216> res = {};
            uint8_t offset = 0;

            std::memcpy(res.data() + offset, &nonce, 8);
            offset += 8;
            std::memcpy(res.data() + offset, gas_price.data(), 16);
            offset += 16;
            std::memcpy(res.data() + offset, &gas_limit, 8);
            offset += 8;
            std::memcpy(res.data() + offset, to.data(), 20);
            offset += 20;
            std::memcpy(res.data() + offset, data.value().data(), 178);
            offset += 178;

            return res;
        } else {
            std::array<uint8_t, 38> res = {};
            uint8_t offset = 0;

            std::memcpy(res.data() + offset, &nonce, 8);
            offset += 8;
            std::memcpy(res.data() + offset, gas_price.data(), 16);
            offset += 16;
            std::memcpy(res.data() + offset, &gas_limit, 8);
            offset += 8;
            std::memcpy(res.data() + offset, to.data(), 20);
            offset += 20;

            return res;
        }
    }
};

