module;

#include <array>

export module signer;

export {
    extern "C" {
        typedef struct {
            std::array<unsigned int, 8> r;
            std::array<unsigned int, 8> s;
            std::array<unsigned int, 16> v;
        } Signature;

        Signature sign_transaction(
            char* ptr,
            std::array<char, 32> private_key,
            unsigned int chain_id
        );
    }
}
