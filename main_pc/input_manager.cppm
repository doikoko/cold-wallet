module;

#include <iostream>
#include <string>
#include <print>

export module input_manager;

import serial;
import BEP20;

export class InputManager {
    Serial const serial;
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
            if (user_input == "1") show_menu();
            else if (user_input == "2") {
                Balance const balance = bep20.get_balance();
                std::println("bnb: {}\nusdc: {}", balance.bnb, balance.usdc);
            }
            else continue;

        }
    }
};
