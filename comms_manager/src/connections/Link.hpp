#pragma once

#include "LinkStatus.hpp"
#include "../settings/Settings.hpp"

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

class Link
{
public:
    explicit Link(unsigned int maxLostMessages) : status(maxLostMessages) {}
    virtual ~Link() = default;

    virtual bool Connect(const ConnectionConfig& config) = 0;
    virtual bool Accept(const ConnectionConfig& config) = 0;
    virtual bool Read(std::span<std::uint8_t> buffer, std::size_t& bytesRead) = 0;
    virtual bool Write(std::span<const std::uint8_t> buffer, std::size_t& bytesWritten) = 0;
    virtual bool End() = 0;
    virtual unsigned short LocalPort() const = 0;

    SocketPerformance GetPerformance() const { return status.GetPerformance(); }
    void ReportLostMessage() { status.ReportLostMessage(); }

protected:
    static void ValidateEndpoint(const ConnectionConfig& config)
    {
        if (config.host.empty() || config.localport < 0 || config.localport > 65535 ||
            config.remoteport < 0 || config.remoteport > 65535)
            throw std::invalid_argument("Invalid connection host or port.");
    }

    asio::io_context context;
    LinkStatus status;
};
