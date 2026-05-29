module;

#include <memory>
#include <print>

#include <asio/serial_port_base.hpp>
#include <asio/awaitable.hpp>
#include <asio/serial_port.hpp>
#include <asio/co_spawn.hpp>

export module serial;

export class Serial {
private:
    asio::serial_port port;

    Serial(asio::any_io_executor const& executor, std::string name)
        : port(executor){}
public:
    ~Serial() {
        port.close();
    }

    static asio::awaitable<std::unique_ptr<Serial>> open(std::string port_name) {
        auto const executor = co_await asio::this_coro::executor;

        std::unique_ptr<Serial> instance(new Serial(executor, port_name));

        try {
            std::error_code err;
            instance->port.open(port_name, err);
            if (err) throw std::system_error(
                err, std::format("Failed to open port: {}", port_name)
            );

            using ser_base = asio::serial_port_base;

            instance->port.set_option(ser_base::baud_rate(9600));
            instance->port.set_option(ser_base::character_size(8));
            instance->port.set_option(ser_base::flow_control(ser_base::flow_control::none));
            instance->port.set_option(ser_base::parity(ser_base::parity::none));
            instance->port.set_option(ser_base::stop_bits(ser_base::stop_bits::one));

            co_return instance;
        }
        catch (const std::system_error& e) {
            std::println("received error {}", e.what());
            std::exit(1);
        }
    }
};

