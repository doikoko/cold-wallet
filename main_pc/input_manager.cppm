module;

#include <iostream>
#include <string>

export module input_manager;

export class InputManager {
public:
    static void inf_loop_init() {
        std::string user_input;

        while (user_input != "4") {
            std::getline(std::cin, user_input);
        }
    }
};
