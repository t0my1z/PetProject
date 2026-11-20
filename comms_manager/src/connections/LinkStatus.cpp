#include "LinkStatus.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace
{
template<typename T>
T Saturate(std::uint64_t value)
{
    return static_cast<T>(std::min(value, static_cast<std::uint64_t>(std::numeric_limits<T>::max())));
}

float Percentage(LinkStatus::Clock::duration part, LinkStatus::Clock::duration whole)
{
    if (whole <= LinkStatus::Clock::duration::zero())
        return 0;
    return static_cast<float>(std::clamp(
        100.0 * std::chrono::duration<double>(part).count() /
            std::chrono::duration<double>(whole).count(), 0.0, 100.0));
}
}

LinkStatus::LinkStatus(unsigned int maxLostMessages) : MAX_LOST_MSG(maxLostMessages)
{
    if (MAX_LOST_MSG == 0)
        throw std::invalid_argument("The lost-message threshold must be greater than zero.");
}

SocketPerformance LinkStatus::GetPerformance() const
{
    const double messages = static_cast<double>(readTimes) + static_cast<double>(totalLostMsg);
    return {
        static_cast<int>(status), sysError, readBytes, writeBytes, lostMsg,
        readTimes ? static_cast<float>(static_cast<double>(totalReadBytes) / readTimes) : 0,
        writeTimes ? static_cast<float>(static_cast<double>(totalWriteBytes) / writeTimes) : 0,
        messages ? static_cast<float>(100.0 * totalLostMsg / messages) : 0,
        Saturate<int>(totalReadBytes), Saturate<int>(totalWriteBytes), Saturate<int>(totalLostMsg),
        pctTimeBlocked, Percentage(totalBlocked, Clock::now() - started)
    };
}

void LinkStatus::ReportLostMessage()
{
    if (lostMsg < std::numeric_limits<unsigned int>::max())
        ++lostMsg;
    ++totalLostMsg;
    if (lostMsg >= MAX_LOST_MSG)
    {
        status = LinkState::NOT_RESPONDING;
        sysError = 0;
    }
}

void LinkStatus::SetReady()
{
    status = LinkState::ALIVE_READY;
    sysError = 0;
    lostMsg = 0;
}

void LinkStatus::SetClosed()
{
    status = LinkState::NOT_RESPONDING;
    sysError = 0;
}

void LinkStatus::SetError(LinkState newStatus, const std::error_code& error)
{
    status = newStatus;
    sysError = error.value();
}

void LinkStatus::RecordBlockedTime(Clock::time_point start, Clock::time_point end)
{
    const auto blocked = end - start;
    totalBlocked += blocked;
    pctTimeBlocked = Percentage(blocked, end - lastOperation);
    lastOperation = end;
}

void LinkStatus::RecordRead(std::size_t bytes, Clock::time_point start, bool success)
{
    RecordBlockedTime(start, Clock::now());
    readBytes = Saturate<unsigned int>(bytes);
    totalReadBytes += bytes;
    if (success || bytes)
        ++readTimes;
    if (success)
    {
        SetReady();
    }
}

void LinkStatus::RecordWrite(std::size_t bytes, Clock::time_point start, bool success)
{
    RecordBlockedTime(start, Clock::now());
    writeBytes = Saturate<unsigned int>(bytes);
    totalWriteBytes += bytes;
    if (success || bytes)
        ++writeTimes;
    if (success && lostMsg < MAX_LOST_MSG)
    {
        status = LinkState::ALIVE_READY;
        sysError = 0;
    }
}
