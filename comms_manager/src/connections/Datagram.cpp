#include "Datagram.hpp"
#include "Log.hpp"

#include <algorithm>
#include <array>

Datagram::Datagram(unsigned int maxLostMessages)
    : Link(maxLostMessages), socket(context)
{

}

bool Datagram::Connect(const ConnectionConfig& config)
{
    ValidateEndpoint(config);

    if (!End())
        return false;
    if (config.remoteport == 0)
    {
        status.SetError(LinkState::ERROR_CONNECT, asio::error::invalid_argument);
        return false;
    }

    asio::error_code error;
    asio::ip::udp::resolver resolver(context);
    const auto endpoints = resolver.resolve(config.host, std::to_string(config.remoteport), error);
    if (error)
    {
        status.SetError(LinkState::ERROR_CONNECT, error);
        return false;
    }

    LinkState failure = LinkState::ERROR_CONNECT;
    for (const auto& entry : endpoints)
    {
        socket.open(entry.endpoint().protocol(), error);
        if (error)
        {
            failure = LinkState::ERROR_SOCKET;
            continue;
        }
        socket.bind(asio::ip::udp::endpoint(entry.endpoint().protocol(), config.localport), error);
        if (error)
            failure = LinkState::ERROR_BIND;
        if (!error)
        {
            // Set socket to non-blocking mode
            socket.non_blocking(true, error);
            if (error)
            {
                asio::error_code closeError;
                socket.close(closeError);
                continue;
            }
            remote_endpoint = entry.endpoint();
            status.SetReady();
            log_info("Datagram ready to send to {}:{}", entry.endpoint().address().to_string(), entry.endpoint().port());
            return true;
        }
        asio::error_code closeError;
        socket.close(closeError);
        if (closeError)
        {
            status.SetError(LinkState::ERROR_SOCKET, closeError);
            return false;
        }
    }
    status.SetError(failure, error ? error : asio::error::host_not_found);
    return false;
}

bool Datagram::Accept(const ConnectionConfig&)
{
    if (!End())
        return false;
    status.SetError(LinkState::ERROR_ACCEPT, asio::error::operation_not_supported);
    return false;
}

bool Datagram::Read(std::span<std::uint8_t> buffer, std::size_t& bytesRead)
{
    asio::error_code error;
    const auto start = LinkStatus::Clock::now();
    std::array<std::uint8_t, 65536> datagram;
    asio::ip::udp::endpoint sender_endpoint;
    const auto received = socket.receive_from(asio::buffer(datagram), sender_endpoint, 0, error);
    bytesRead = std::min(received, buffer.size());
    std::copy_n(datagram.begin(), bytesRead, buffer.begin());
    
    // Non-blocking socket: would_block just means no data available, not an error
    if (error == asio::error::would_block || error == asio::error::try_again)
    {
        bytesRead = 0;
        return true;  // Not an error, just no data available
    }
    
    // Connection reset or port unreachable are okay - remote end just isn't ready
    if (error == asio::error::connection_reset || 
        error == asio::error::connection_refused ||
        error == asio::error::host_unreachable)
    {
        bytesRead = 0;
        return true;  // Don't treat connection errors as fatal in startup
    }
    
    if (!error && received > buffer.size())
        error = asio::error::message_size;
    status.RecordRead(bytesRead, start, !error);
    if (error)
        status.SetError(LinkState::ERROR_RECV, error);
    return !error;
}

bool Datagram::Write(std::span<const std::uint8_t> buffer, std::size_t& bytesWritten)
{
    asio::error_code error;
    const auto start = LinkStatus::Clock::now();
    bytesWritten = socket.send_to(asio::buffer(buffer.data(), buffer.size()), remote_endpoint, 0, error);
    status.RecordWrite(bytesWritten, start, !error);
    if (error)
        status.SetError(LinkState::ERROR_SEND, error);
    return !error;
}

bool Datagram::End()
{
    asio::error_code error;
    socket.close(error);
    if (error)
    {
        status.SetError(LinkState::ERROR_SOCKET, error);
        return false;
    }
    status.SetClosed();
    return true;
}

unsigned short Datagram::LocalPort() const
{
    return socket.local_endpoint().port();
}
