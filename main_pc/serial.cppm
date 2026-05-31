module;

#include <print>

#include <asio/serial_port_base.hpp>
#include <asio/serial_port.hpp>
#include <asio/buffer.hpp>

export module serial;

import commands;

export class Serial {
    asio::serial_port port;

    void sync() {
        uint8_t read_data = 0;
        asio::mutable_buffer const read_buf = asio::buffer(&read_data, 1);

        uint8_t sync_pc_raw = static_cast<uint8_t>(Commands::SYNC_PC);
        asio::mutable_buffer const sync_pc = asio::buffer(&sync_pc_raw, 1);

        while (
            *static_cast<uint8_t*>(read_buf.data()) !=
            static_cast<uint8_t>(Commands::SYNC_MCU))
        {
            port.write_some(sync_pc);
            port.read_some(read_buf);
        }
    }

public:
    Serial(asio::io_context& context, std::string port_name) :
        port(context)
    {
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

    std::string get_address_from_mcu() {
        char addr_raw[42] = {};
        asio::mutable_buffer const addr = asio::buffer(addr_raw, 42);

        sync();
        port.read_some(addr);

        return addr_raw;
    }
};

