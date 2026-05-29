#include <iostream>
#include <print>
#include <memory>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

import serial;

asio::awaitable<int> async_main() {
    std::println("input serial port\ne.g. '/dev/ttyUSB0");

    std::string port_name;
    std::getline(std::cin, port_name);

    std::unique_ptr serial(co_await Serial::open(std::move(port_name)));
    co_return 1;
}

int main() {
    asio::io_context io_ctx;

    asio::co_spawn(io_ctx, async_main(), asio::detached);
    io_ctx.run();
}