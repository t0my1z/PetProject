#pragma once

#include <cstdint>
#include <format>
#include <string_view>

// All available log levels
enum class LogLevel : uint8_t {
    Debug,
    Info,
    Warn,
    Error,
};

// 16 standard ANSI colors
enum class Color : uint8_t {
    None,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
};

namespace detail {

void log_impl(LogLevel level, Color color, std::string_view fmt_str);

template <typename... Args>
void log(LogLevel level, Color color, std::format_string<Args...> fmt,
    // NOLINTNEXTLINE
    Args&&... args) {
    std::string msg;
    std::vformat_to(std::back_inserter(msg), fmt.get(),
    std::make_format_args(args...));
    log_impl(level, color, msg);
}

} // namespace detail

// Utility functions. You should use these instead of the ones above.
template <typename... Args>
void log_debug(std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Debug, Color::None, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_debug(Color color, std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Debug, color, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Info, Color::None, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(Color color, std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Info, color, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_warn(std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Warn, Color::None, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_warn(Color color, std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Warn, color, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Error, Color::None, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_error(Color color, std::format_string<Args...> fmt, Args&&... args) {
    detail::log(LogLevel::Error, color, fmt, std::forward<Args>(args)...);
}

// Set the minimum log level. Log messages below this level are dropped
void set_log_level(LogLevel level);

// Set an optional log file
void set_log_file(std::string_view filename);

// Enable or disable multi-threaded logging. Must be called before spawning
// threads that log concurrently.
void set_log_thread_safe(bool enabled);