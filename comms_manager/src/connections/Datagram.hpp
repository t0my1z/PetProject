#pragma once

#include "Link.hpp"

class Datagram final : public Link
{
public:
    explicit Datagram(unsigned int maxLostMessages = 3);

    bool Connect(const ConnectionConfig& config) override;
    bool Accept(const ConnectionConfig& config) override;
    bool Read(std::span<std::uint8_t> buffer, std::size_t& bytesRead) override;
    bool Write(std::span<const std::uint8_t> buffer, std::size_t& bytesWritten) override;
    bool End() override;
    unsigned short LocalPort() const override;

private:
    asio::ip::udp::socket socket;
};
