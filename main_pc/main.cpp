#include <iostream>
#include <print>
#include <memory>
#include <asio/io_context.hpp>

import serial;
import input_manager;
import logger_pc;
import BEP20;
import result;

int main() {
    std::println("input serial port\ne.g. '/dev/ttyUSB0");

    Logger::instance("main", "../../logs.log");

    std::string port_name;
    std::getline(std::cin, port_name);

    try {
        asio::io_context ctx;
        Serial serial(ctx, port_name);

        Logger::instance().write(std::format("connected to {}", port_name), LogType::Info);

        std::string address = serial.get_address_from_mcu();

        BEP20 bep20(ctx, std::move(address));

        InputManager manager(std::move(serial), std::move(bep20));

        manager.show_menu();
        manager.inf_loop_init();
    } catch (std::system_error const& e) {
        Logger::instance().write(
            std::format(
                "an error occurred while connecting to the serial port. {}", std::move(port_name)
                ), LogType::Error
            );

        return 1;
    }

    return 0;
}