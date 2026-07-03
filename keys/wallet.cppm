module;

#include <string>
#include <print>
#include <format>
#include <cstring>

#include <TWHDWallet.h>
#include <TWPrivateKey.h>
#include <TWCoinType.h>
#include <TWString.h>
#include <TWData.h>

export module wallet;

export class Wallet{
public:
    std::string mnemonic_str{};
    std::array<uint8_t, 32> private_key{};
    std::array<uint8_t, 20> address{};

    void generate(){
        std::println("start generation keys");

        TWHDWallet* wallet = TWHDWalletCreate(128, TWStringCreateWithUTF8Bytes(""));
        if (!wallet) {
            throw std::runtime_error("failed to generate HD Wallet");
        }

        TWString* tw_mnemonic = TWHDWalletMnemonic(wallet);
        mnemonic_str = TWStringUTF8Bytes(tw_mnemonic);
        TWStringDelete(tw_mnemonic);

        TWPrivateKey* pr_key = TWHDWalletGetKeyForCoin(wallet, TWCoinTypeEthereum);

        TWData* pk_data = TWPrivateKeyData(pr_key);

        const uint8_t* pk_bytes = TWDataBytes(pk_data);

        std::memcpy(private_key.data(), pk_bytes, private_key.size());

        TWDataDelete(pk_data);
        TWPrivateKeyDelete(pr_key);

        TWString* tw_address = TWHDWalletGetAddressForCoin(wallet, TWCoinTypeEthereum);

        std::string address_str = TWStringUTF8Bytes(tw_address);

        if (address_str.starts_with("0x"))
            address_str = address_str.substr(2);

        for (uint8_t i = 0; i < address.size(); i++)
            address[i] = std::stoi(address_str.substr(i * 2, 2), nullptr, 16);

        TWStringDelete(tw_address);
        TWHDWalletDelete(wallet);
    }
};