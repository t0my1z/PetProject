#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <system_error>

enum class LinkState
{
    ERROR_SOCKET = -1,
    ERROR_BIND = -2,
    ERROR_ACCEPT = -3,
    ERROR_LISTEN = -4,
    ERROR_CONNECT = -5,
    ERROR_RECV = -6,
    ERROR_SEND = -7,
    ERROR_PACK = -8,
    ERROR_UNPACK = -9,
    ERROR_WSASTARTUP = -10,
    NOT_RESPONDING = 0,
    ALIVE_READY = 1
};

struct SocketPerformance
{
    int status;
    int sysError;
    unsigned int readBytes;
    unsigned int writeBytes;
    unsigned int lostMsg;
    float meanRead;
    float meanWrite;
    float pctLost;
    int totalRead;
    int totalWrite;
    int totalLost;
    float pctTimeBlocked;
    float meanPctTimeBlocked;
};

class LinkStatus
{
public:
    using Clock = std::chrono::steady_clock;

    explicit LinkStatus(unsigned int maxLostMessages = 3);

    SocketPerformance GetPerformance() const;
    void ReportLostMessage();

    void SetReady();
    void SetClosed();
    void SetError(LinkState status, const std::error_code& error);
    void RecordRead(std::size_t bytes, Clock::time_point start, bool success);
    void RecordWrite(std::size_t bytes, Clock::time_point start, bool success);

private:
    void RecordBlockedTime(Clock::time_point start, Clock::time_point end);

    const unsigned int MAX_LOST_MSG;
    LinkState status = LinkState::NOT_RESPONDING;
    int sysError = 0;
    unsigned int readBytes = 0;
    unsigned int writeBytes = 0;
    unsigned int lostMsg = 0;
    std::uint64_t readTimes = 0;
    std::uint64_t writeTimes = 0;
    std::uint64_t totalLostMsg = 0;
    std::uint64_t totalReadBytes = 0;
    std::uint64_t totalWriteBytes = 0;
    float pctTimeBlocked = 0;
    Clock::time_point started = Clock::now();
    Clock::time_point lastOperation = started;
    Clock::duration totalBlocked{};
};
