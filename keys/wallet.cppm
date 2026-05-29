module;

#include <string>
#include <print>
#include <format>

#include <TWHDWallet.h>
#include <TWPrivateKey.h>
#include <TWCoinType.h>
#include <TWString.h>
#include <TWData.h>

export module wallet;

export class Wallet{
    TWHDWallet* wallet_ptr = nullptr;

public:
    std::string mnemonic_str{};
    std::string private_key_hex{};
    std::string address_str{};

    ~Wallet(){
        if (wallet_ptr) {
            TWHDWalletDelete(wallet_ptr);
        }
    }

    void generate(){
        std::println("start generation keys");

        TWHDWallet* wallet = TWHDWalletCreate(128, TWStringCreateWithUTF8Bytes(""));
        if (!wallet) {
            throw std::runtime_error("failed to generate HD Wallet");
        }
        wallet_ptr = wallet;

        TWString* tw_mnemonic = TWHDWalletMnemonic(wallet);
        mnemonic_str = TWStringUTF8Bytes(tw_mnemonic);
        TWStringDelete(tw_mnemonic);

        TWPrivateKey* private_key = TWHDWalletGetKeyForCoin(wallet, TWCoinTypeEthereum);

        TWData* pk_data = TWPrivateKeyData(private_key);
        for (size_t i = 0; i < TWDataSize(pk_data); ++i) {
            private_key_hex += std::format("{:02x}", TWDataGet(pk_data, i));
        }
        TWDataDelete(pk_data);

        TWString* tw_address = TWHDWalletGetAddressForCoin(wallet, TWCoinTypeEthereum);
        address_str = TWStringUTF8Bytes(tw_address);
        TWStringDelete(tw_address);

        TWPrivateKeyDelete(private_key);
    }
};
