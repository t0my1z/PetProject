#include "Connection.hpp"
#include "Datagram.hpp"
#include "Stream.hpp"

#include <stdexcept>
#include <utility>

Connection::Connection(ConnectionConfig config, unsigned int maxLostMessages)
    : config(std::move(config))
{
    if (this->config.host.empty())
        throw std::invalid_argument("Connection host must not be empty.");
    if (this->config.localport < 0 || this->config.localport > 65535 ||
        this->config.remoteport < 0 || this->config.remoteport > 65535)
        throw std::invalid_argument("Connection ports must be between 0 and 65535.");
    if (this->config.inbuffersize <= 0 || this->config.outbuffersize <= 0)
        throw std::invalid_argument("Connection buffer sizes must be positive.");

    switch (this->config.type)
    {
    case ConnectionType::UDP:
        link = std::make_unique<Datagram>(maxLostMessages);
        break;
    case ConnectionType::TCP:
        link = std::make_unique<Stream>(maxLostMessages);
        break;
    default:
        throw std::invalid_argument("Unsupported connection type.");
    }
    inputBuffer.reserve(this->config.inbuffersize);
    outputBuffer.reserve(this->config.outbuffersize);
}

bool Connection::Connect()
{
    mode = Mode::Connect;
    return link->Connect(config);
}

bool Connection::Accept()
{
    mode = Mode::Accept;
    return link->Accept(config);
}

bool Connection::Reconnect()
{
    switch (mode)
    {
    case Mode::Connect:
        return link->Connect(config);
    case Mode::Accept:
        return link->Accept(config);
    case Mode::None:
        throw std::logic_error("Connect or Accept must be called before Reconnect.");
    }
    throw std::logic_error("Invalid connection mode.");
}

bool Connection::End()
{
    return link->End();
}

bool Connection::Read()
{
    inputBuffer.resize(config.inbuffersize);
    std::size_t bytesRead = 0;
    const bool success = link->Read(inputBuffer, bytesRead);
    inputBuffer.resize(bytesRead);
    return success;
}

bool Connection::Write()
{
    std::size_t bytesWritten = 0;
    return link->Write(outputBuffer, bytesWritten);
}

void Connection::SetOutputBuffer(std::span<const std::uint8_t> data)
{
    if (data.size() > static_cast<std::size_t>(config.outbuffersize))
        throw std::length_error("Output data exceeds the configured buffer size.");
    outputBuffer = std::vector<std::uint8_t>(data.begin(), data.end());
}
