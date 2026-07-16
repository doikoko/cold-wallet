module;

#include "main.h"

#include <array>
#include <cstring>

export module keys;

constexpr uint8_t addr_len = 20;
constexpr uint8_t pr_key_len = 32;
constexpr uint8_t mnemonic_words = 12;
constexpr uint8_t max_word_len = 9; // with '\0'
constexpr uint8_t mnemonic_len = max_word_len * mnemonic_words + 1; // with '\0'
constexpr uint8_t hex_len = 45; // with "0x", 2'\n' for correct display and '\0'

export class Keys {
public:
    std::array<char, addr_len> addr{};
    std::array<char, hex_len> addr_str{'0', 'x'};
    std::array<char, pr_key_len> private_key{};
    std::array<std::array<char, max_word_len>, mnemonic_len> mnemonic{{'\0'}};
    bool _is_first_run;

    Keys() {
        char const* otp_base = reinterpret_cast<char*>(0x1F'FF'78'00);

        std::memcpy(addr.data(), otp_base, addr_len);

        constexpr char hex_chars[] = "0123456789ABCDEF";

        char* addr_ptr = addr_str.data() + 2;

        for (size_t i = 0; i < addr_len; ++i) {
            if (i != 0 && i % 7 == 0) {
                *addr_ptr++ = '\n';
            }

            uint8_t const byte = static_cast<uint8_t>(addr[i]);
            *addr_ptr++ = hex_chars[byte >> 4];
            *addr_ptr++ = hex_chars[byte & 0x0F];
        }

        *addr_ptr = '\0';

        std::memcpy(private_key.data(), otp_base + addr_len, pr_key_len);

        _is_first_run = *(otp_base + addr_len + pr_key_len + mnemonic_len);

        char const* ptr = otp_base + addr_len + pr_key_len;
        for (std::array<char, max_word_len> mnem : mnemonic) {
            for (uint8_t i = 0; i < 9; i++) {
                if (*ptr == '\0') {
                    return;
                } else if (*ptr != ' ') {
                    mnem[i] = *ptr;
                    ptr++;
                } else {
                    ptr++;
                    break;
                }
            }
        }
    }
};