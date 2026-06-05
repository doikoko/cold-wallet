module;

#include <iostream>
#include <string>
#include <print>

export module input_manager;

import serial;
import BEP20;

export class InputManager {
    Serial serial;
    BEP20 bep20;
public:
    explicit InputManager(Serial serial, BEP20 bep20) :
        serial(std::move(serial)),
        bep20(std::move(bep20)){}

    void show_menu() const {
        std::println("to choose an action enter the corresponding number\n"
            "e.g. '1' to show this menu.\n"
            "available functions:\n"
            "1) show this menu\n"
            "2) check current balance\n"
            "3) send transaction\n"
            "4) show address\n"
            "5) exit"
        );
    }

    void inf_loop_init() {
        std::string user_input;

        while (user_input != "5") {
            std::getline(std::cin, user_input);
            if (user_input == "1") {
                show_menu();
            }
            else if (user_input == "2") {
                Balance const balance = bep20.get_balance();
                std::println("bnb: {}\nusdc: {}", balance.bnb, balance.usdc);
                show_menu();
            }
            else if (user_input == "3") {
                std::string to;
                while (true) {
                    std::println("input address");
                    std::getline(std::cin, to);
                    if (to.length() != 42)
                        std::println("invalid format, try again");
                    else
                        break;
                }

                std::string token_str;
                SupportedTokens token;

                while (true) {
                    std::println("input token number from list:\n"
                        "1) bnb\n"
                        "2) usdc\n"
                        "e.g. '1'"
                    );
                    std::getline(std::cin, token_str);

                    if (token_str == "1") {
                        token = SupportedTokens::BNB;
                        break;
                    }
                    else if (token_str == "2") {
                        token = SupportedTokens::USDC;
                        break;
                    }
                    else
                        std::println("invalid format, try again");
                }

                std::string amount_str;
                double amount = 0.0;
                while (true) {
                    std::println("input amount");
                    std::getline(std::cin, amount_str);
                    try {
                        amount = std::stod(amount_str);
                        break;
                    }
                    catch (...){
                        std::println("invalid format, try again");
                    }

                    bep20.send_transaction();
                }

                show_menu();
            }
            else if (user_input == "4") {
                std::println("address: {}", bep20.get_address_cached());
                std::println("check address on display");

                serial.show_address_mcu();

                show_menu();
            }
            else continue;
        }
    }
};
