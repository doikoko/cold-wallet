module;

#include <memory>
#include <print>

#include <asio/serial_port_base.hpp>
#include <asio/serial_port.hpp>

export module serial;

export class Serial {
    asio::serial_port port;

public:
    Serial(asio::io_context context, std::string port_name) :
        port(context) {
        try {
            std::error_code err;
            asio::error_code const a_err = port.open(port_name, err);
            if (err || a_err) throw std::system_error(
                err, std::format("Failed to open port: {}", port_name)
            );

            using ser_base = asio::serial_port_base;

            port.set_option(ser_base::baud_rate(9600));
            port.set_option(ser_base::character_size(8));
            port.set_option(ser_base::flow_control(ser_base::flow_control::none));
            port.set_option(ser_base::parity(ser_base::parity::none));
            port.set_option(ser_base::stop_bits(ser_base::stop_bits::one));
        }
        catch (const std::system_error& e) {
            std::println("received error {}", e.what());
            std::exit(1);
        }
    }

    ~Serial() {
        port.close();
    }
};

