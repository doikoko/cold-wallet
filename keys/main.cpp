#include <format>
#include <string>
#include <print>
#include <cstdlib>

import wallet;

int main() {
    try {
        Wallet wallet;
        wallet.generate();

        std::string command = std::format(
            "STM32_Programmer_CLI -c port=usb1 \
            -otp write word=0 fvalue={}{}{}",
            wallet.address_str, 
            wallet.private_key_hex,
            wallet.mnemonic_str
        );

        if (std::system(command.data())){
            std::println(stderr, "error occured");
            return 1;
        }
    } catch (const std::exception& e) {
        std::println(stderr, "error: {}", e.what());
        return 1;
    }
    return 0;
}