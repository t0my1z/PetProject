#include "Stream.hpp"
#include "Log.hpp"

Stream::Stream(unsigned int maxLostMessages)
    : Link(maxLostMessages), socket(context), acceptor(context)
{
}

bool Stream::Connect(const ConnectionConfig& config)
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
    asio::ip::tcp::resolver resolver(context);
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
        socket.bind(asio::ip::tcp::endpoint(entry.endpoint().protocol(), config.localport), error);
        if (error)
            failure = LinkState::ERROR_BIND;
        else
        {
            socket.connect(entry.endpoint(), error);
            failure = LinkState::ERROR_CONNECT;
        }
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
            status.SetReady();
            log_info("Stream connected to {}:{}", entry.endpoint().address().to_string(), entry.endpoint().port());
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

bool Stream::Accept(const ConnectionConfig& config)
{
    ValidateEndpoint(config);
    if (!End())
        return false;
    asio::error_code error;
    asio::ip::tcp::resolver resolver(context);
    const auto endpoints = resolver.resolve(config.host, std::to_string(config.remoteport), error);
    if (error)
    {
        status.SetError(LinkState::ERROR_ACCEPT, error);
        return false;
    }
    LinkState failure = LinkState::ERROR_ACCEPT;
    for (const auto& entry : endpoints)
    {
        acceptor.open(entry.endpoint().protocol(), error);
        if (error)
        {
            failure = LinkState::ERROR_SOCKET;
            continue;
        }
        acceptor.bind(asio::ip::tcp::endpoint(entry.endpoint().protocol(), config.localport), error);
        if (error)
            failure = LinkState::ERROR_BIND;
        else
        {
            acceptor.listen(asio::socket_base::max_listen_connections, error);
            failure = LinkState::ERROR_LISTEN;
        }
        if (!error)
        {
            acceptor.accept(socket, error);
            failure = LinkState::ERROR_ACCEPT;
            if (!error)
            {
                // Set socket to non-blocking mode
                socket.non_blocking(true, error);
                if (error)
                {
                    acceptor.close(error);
                    status.SetError(LinkState::ERROR_SOCKET, error);
                    return false;
                }
                acceptor.close(error);
                if (error)
                {
                    status.SetError(LinkState::ERROR_SOCKET, error);
                    return false;
                }
                status.SetReady();
                return true;
            }
        }
        asio::error_code closeError;
        acceptor.close(closeError);
        if (closeError)
        {
            status.SetError(LinkState::ERROR_SOCKET, closeError);
            return false;
        }
    }
    status.SetError(failure, error ? error : asio::error::host_not_found);
    return false;
}

bool Stream::Read(std::span<std::uint8_t> buffer, std::size_t& bytesRead)
{
    asio::error_code error;
    const auto start = LinkStatus::Clock::now();
    bytesRead = socket.read_some(asio::buffer(buffer.data(), buffer.size()), error);
    
    // Non-blocking socket: would_block just means no data available, not an error
    if (error == asio::error::would_block || error == asio::error::try_again)
    {
        bytesRead = 0;
        return true;  // Not an error, just no data available
    }
    
    // Connection reset or refused are okay - remote end just isn't ready
    if (error == asio::error::connection_reset || 
        error == asio::error::connection_refused)
    {
        bytesRead = 0;
        return true;  // Don't treat connection errors as fatal in startup
    }
    
    status.RecordRead(bytesRead, start, !error);
    if (error)
        status.SetError(LinkState::ERROR_RECV, error);
    return !error;
}

bool Stream::Write(std::span<const std::uint8_t> buffer, std::size_t& bytesWritten)
{
    asio::error_code error;
    const auto start = LinkStatus::Clock::now();
    bytesWritten = 0;
    if (!socket.is_open())
        error = asio::error::bad_descriptor;
    else
        bytesWritten = asio::write(socket, asio::buffer(buffer.data(), buffer.size()), error);
    status.RecordWrite(bytesWritten, start, !error);
    if (error)
        status.SetError(LinkState::ERROR_SEND, error);
    return !error;
}

bool Stream::End()
{
    asio::error_code socketError;
    asio::error_code acceptorError;
    socket.close(socketError);
    acceptor.close(acceptorError);
    if (socketError || acceptorError)
    {
        status.SetError(LinkState::ERROR_SOCKET, socketError ? socketError : acceptorError);
        return false;
    }
    status.SetClosed();
    return true;
}

unsigned short Stream::LocalPort() const
{
    return socket.local_endpoint().port();
}
