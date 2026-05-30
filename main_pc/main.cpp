#include <iostream>
#include <print>
#include <memory>
#include <asio/io_context.hpp>

import serial;
import input_manager;
import logger;

int main() {
    std::println("input serial port\ne.g. '/dev/ttyUSB0");

    Logger::instance("main", "../../logger.log");

    std::string port_name;
    std::getline(std::cin, port_name);

    Serial serial(asio::io_context{}, std::move(port_name));

    std::println("to choose an action enter the corresponding number\n"
        "e.g. '1' to show this menu.\n"
        "available functions:\n"
        "1) show this menu\n"
        "2) check current balance\n"
        "3) send transaction\n"
        "4) exit"
    );

    InputManager::inf_loop_init();

    return 0;
}