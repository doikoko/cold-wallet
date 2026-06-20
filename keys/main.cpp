#include <format>
#include <string>
#include <print>
#include <cstdlib>
#include <iostream>
#include <fstream>

import wallet;

constexpr std::string tmp_file = "tmp.txt";
constexpr uint32_t otp_base = 0x1F'FF'78'00;

int main() {
    std::println(
        "choose how to write keys to OTP memory\n"
        "1) SWD\n"
        "2) USART"
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

        std::ofstream file(tmp_file);
        file << wallet.address_str
            << wallet.private_key_hex
            << wallet.mnemonic_str
            << '\0';

        if (mode == "1") {
            std::println("input serial port\ne.g. '/dev/ttyUSB0");

            std::string port_name;
            std::getline(std::cin, port_name);

            command = std::format(
                "STM32_Programmer_CLI -c port={} "
                "-w {} {}",
                port_name,
                tmp_file,
                otp_base
            );
        } else if (mode == "2") {
            command = std::format(
                "STM32_Programmer_CLI -c port=SWD "
                "-w {} {}",
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