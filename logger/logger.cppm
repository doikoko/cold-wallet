module;

#include <fstream>
#include <string>
#include <print>
#include <chrono>
#include <format>

export module logger;

export enum class LogType {
    Info,
    Warning,
    Error
};
export class Logger {
    std::string const process_name;
    std::ofstream stream;

    Logger(std::string process_name, std::string const& filename) :
        process_name(std::move(process_name)),
        stream(filename, std::ios::app) {std::println("debug");}

    ~Logger() {
        stream.close();
    }

    std::string get_time() {
        auto const now = std::chrono::system_clock::now();
        auto const now_seconds = std::chrono::floor<std::chrono::seconds>(now);

        auto const local_time = std::chrono::zoned_time{
            std::chrono::current_zone(),
            now_seconds
        };

        return std::format("{}", local_time);
    }
public:
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    static Logger& instance(std::string process_name = "", std::string const& filename = "") {
        static Logger instance(std::move(process_name), filename);
        return instance;
    }

    void write(std::string text, LogType const type) {
        std::string log_type;
        switch (type) {
            case (LogType::Info):
                log_type = "INFO   ";
                break;
            case (LogType::Warning):
                log_type = "WARNING";
                break;
            case (LogType::Error):
                log_type = "ERROR  ";
                break;
        }

        stream << std::format(
            "{} | {} | {} | {}\n",
            get_time(),
            process_name,
            log_type,
            std::move(text)
        );
    }
};