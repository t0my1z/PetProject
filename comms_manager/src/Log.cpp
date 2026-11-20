#include <Log.hpp>

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace {

constexpr const char* colreset = "\033[0m";

bool supports_ansi_colors() {
#ifdef _WIN32
    static const bool supported = [] {
        const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        return output != INVALID_HANDLE_VALUE
            && GetConsoleMode(output, &mode)
            && SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }();
    return supported;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

const char* colcode(Color color) {
    switch (color) {
        case Color::None: return "";
        case Color::Black: return "\033[0;30m";
        case Color::Red: return "\033[0;31m";
        case Color::Green: return "\033[0;32m";
        case Color::Yellow: return "\033[0;33m";
        case Color::Blue: return "\033[0;34m";
        case Color::Magenta: return "\033[0;35m";
        case Color::Cyan: return "\033[0;36m";
        case Color::White: return "\033[0;37m";
        case Color::BrightBlack: return "\033[1;30m";
        case Color::BrightRed: return "\033[1;31m";
        case Color::BrightGreen: return "\033[1;32m";
        case Color::BrightYellow: return "\033[1;33m";
        case Color::BrightBlue: return "\033[1;34m";
        case Color::BrightMagenta: return "\033[1;35m";
        case Color::BrightCyan: return "\033[1;36m";
        case Color::BrightWhite: return "\033[1;37m";
    }
    return "";
}

const char* levelstr(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO ";
        case LogLevel::Warn: return "WARN ";
        case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

std::atomic<LogLevel> g_min_level{LogLevel::Debug};
std::atomic<bool> g_threadsafe{false};
std::mutex g_mutex;
std::optional<std::ofstream> g_log_file;

} // anonymous namespace

namespace detail {

void log_impl(LogLevel level, Color color, std::string_view fmt_str) 
{
    if (level < g_min_level.load(std::memory_order_relaxed)) return;
    
    namespace sc = std::chrono;
    const auto now = sc::system_clock::now();
    const auto hms = sc::hh_mm_ss{now - sc::floor<sc::days>(now)};
    const char* lvl = levelstr(level);
    const bool use_colors = supports_ansi_colors();
    const char* lvl_color = use_colors ? colcode(level == LogLevel::Debug ? Color::Blue
        : level == LogLevel::Info ? Color::Green
        : level == LogLevel::Warn ? Color::Yellow
        : Color::Red) : "";
    const char* lvl_reset = use_colors ? colreset : "";

    const char* msg_color = use_colors ? colcode(color) : "";
    const char* msg_reset = use_colors && color != Color::None ? colreset : "";
    {
        std::unique_lock lock{g_mutex, std::defer_lock};
        if (g_threadsafe.load(std::memory_order_relaxed)) {
        lock.lock();
        }
        // Format a colored log message for standard output
        std::format_to(std::ostreambuf_iterator<char>(std::cout),
        "[{}] [{}{}{}] {}{}{}\n",
        hms, lvl_color, lvl, lvl_reset, msg_color, fmt_str, msg_reset);
        std::ignore = std::fflush(stdout);
        // Optionally log to a file without ANSI escape codes
        if (g_log_file && g_log_file->is_open()) {
        std::format_to(std::ostreambuf_iterator<char>(*g_log_file),
        "[{}] [{}] {}\n", hms, lvl, fmt_str);
        g_log_file->flush();
        }
    }
}

} // namespace detail

void set_log_level(LogLevel level) {
    g_min_level.store(level, std::memory_order_relaxed);
}

void set_log_file(std::string_view filename) {
    std::lock_guard lock{g_mutex};
    if (filename.empty()) {
    g_log_file.reset();
    } else {
    g_log_file.emplace(std::string{filename}, std::ios::app);
    }
}

void set_log_threadsafe(bool enabled) {
    g_threadsafe.store(enabled, std::memory_order_relaxed);
}