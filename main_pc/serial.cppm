module;

#include <iomanip>

#include <asio/serial_port_base.hpp>
#include <asio/serial_port.hpp>
#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

export module serial;

import commands;
import result;

constexpr uint8_t len_msg_size = 6;
export class Serial {
    asio::serial_port port;

    void sync() {
        uint8_t data = 0;

        while (data != command_sync_mcu){
            asio::write(port, asio::buffer(&command_sync_pc, 1));
            asio::read(port, asio::buffer(&data, 1));
        }
    }

    [[nodiscard]] Result check_res() {
        uint8_t data = 0;

        asio::read(port, asio::buffer(&data, 1));
        Result const result = data == command_ok ? Result::Ok : Result::Err;

        asio::write(port, asio::buffer(&command_sync_pc, 1));

        return result;
    }

    void send_res(Result const res) {
        uint8_t data = 0;
        uint8_t const result = res == Result::Ok ? command_ok : command_err;

        while (data != command_sync_mcu) {
            asio::write(port, asio::buffer(&result, 1));
            asio::read(port, asio::buffer(&data, 1));
        }
    }
protected:
    [[nodiscard]] std::optional<std::string> get_address_from_mcu() {
        if (send_command(command_get_addr) == Result::Err)
            return std::nullopt;

        std::optional const opt_addr = receive();
        if (!opt_addr.has_value())
            return std::nullopt;

        std::vector<char> const& addr = opt_addr.value();
        std::ostringstream oss;

        oss << "0x";
        oss << std::hex << std::uppercase;

        for (char const ch : addr)
            oss << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(ch));

        return oss.str();
    }
public:
    Serial(asio::io_context& context, std::string const& port_name) :
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

    [[nodiscard]] Result send(std::span<const char> const msg) {
        // 1 start byte | 4 data bytes | 1 end byte
        uint8_t len_msg[len_msg_size];
        len_msg[0] = command_start_byte;
        uint32_t const msg_size = static_cast<uint32_t>(msg.size());

        std::memcpy(len_msg + 1, &msg_size, 4);
        len_msg[5] = command_end_byte;

        uint32_t const data_msg_size = msg_size + 2;
        uint8_t data_msg[data_msg_size];

        // 1 start byte | data | 1 end byte
        data_msg[0] = command_start_byte;
        std::memcpy(data_msg + 1, msg.data(), msg.size());
        data_msg[data_msg_size - 1] = command_end_byte;

        uint8_t attempts = 0;

        sync();
        do {
            asio::write(port, asio::buffer(len_msg, len_msg_size));
            if (++attempts >= 3)
                return Result::Err;
        } while(check_res() != Result::Ok);

        attempts = 0;

        sync();
        do {
            asio::write(port, asio::buffer(data_msg, data_msg_size));
            if (++attempts >= 3)
                return Result::Err;
        } while (check_res() != Result::Ok);

        return Result::Ok;
    }

    [[nodiscard]] std::optional<std::vector<char>> receive() {
        uint8_t len_msg[len_msg_size] = {};
        uint8_t attempts = 0;

        sync();
        while (true) {
            asio::read(port, asio::buffer(len_msg, len_msg_size));
            if (++attempts >= 3)
                return std::nullopt;
            if (len_msg[0] == command_start_byte && len_msg[len_msg_size - 1] == command_end_byte) {
                send_res(Result::Ok);
                break;
            }
            else
                send_res(Result::Err);
        }
        uint32_t msg_size;
        std::memcpy(&msg_size, len_msg + 1, 4);

        std::vector<char> msg;
        msg.resize(msg_size);

        uint8_t buf[msg_size + 2];
        attempts = 0;

        sync();
        while (true) {
            asio::read(port, asio::buffer(buf, msg_size + 2));
            if (++attempts >= 3)
                return std::nullopt;
            if (buf[0] == command_start_byte && buf[msg_size + 1] == command_end_byte) {
                send_res(Result::Ok);
                break;
            }
            else
                send_res(Result::Err);
        }

        std::memcpy(msg.data(), buf + 1, msg_size);
        return msg;
    }

    [[nodiscard]] Result send_command(char const command) {
        if (command < 0x47 || command > 0x4F)
            return Result::Err;

        return send(std::span(&command, 1));
    }

    [[nodiscard]] std::optional<char> receive_command() {
        std::optional const res = receive();

        if (!res.has_value())
            return std::nullopt;

        char const command = res.value()[0];

        if (command < 0x46 || command > 0x4F)
            return std::nullopt;

        return command;
    }

    [[nodiscard]] Result show_address_mcu() {
        auto a = send_command(command_show_addr);
        if (a == Result::Err) {
            return Result::Err;
        }

        std::optional const command = receive_command();
        if (!command.has_value())
            return Result::Err;

        if (command.value() != command_finish)
            return Result::Err;

        return Result::Ok;
    }
};