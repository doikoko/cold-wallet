#include <format>
#include <string>
#include <print>
#include <cstdlib>
#include <iostream>
#include <fstream>

import wallet;

constexpr std::string tmp_file = "./tmp.bin";
constexpr uint32_t otp_base = 0x1F'FF'78'00;

int main() {
    std::println(
        "choose how to write keys to OTP memory\n"
        "1) USART\n"
        "2) SWD"
    );
    std::string mode;

    while (true) {
        std::getline(std::cin, mode);
        if (mode == "1" || mode == "2")
            break;

        std::println("incorrect input, expecting '1' or '2'");
    }

    try {
        Wallet wallet;
        wallet.generate();

        std::string command;

        {
            std::ofstream file(tmp_file);
            file.write(reinterpret_cast<const char*>(wallet.address.data()), wallet.address.size());
            file.write(reinterpret_cast<const char*>(wallet.private_key.data()), wallet.private_key.size());

            file.write(wallet.mnemonic_str.data(), wallet.mnemonic_str.size());
        }

        if (mode == "1") {
            std::println("input serial port\ne.g. '/dev/ttyUSB0");

            std::string port_name;
            std::getline(std::cin, port_name);

            command = std::format(
                "STM32_Programmer_CLI -c port={} "
                "-d {} {}",
                port_name,
                tmp_file,
                otp_base
            );
        } else if (mode == "2") {
            command = std::format(
                "STM32_Programmer_CLI -c port=SWD "
                "-d {} {}",
                tmp_file,
                otp_base
            );
        }
        int const res = std::system(command.data());
        std::remove(tmp_file.data());
        if (res) {
           throw std::exception();
        }
    } catch (const std::exception& e) {
        std::println(stderr, "error: {}", e.what());
        return 1;
    }

    return 0;
}