#pragma once

#include "Link.hpp"

#include <memory>
#include <span>
#include <vector>

class Connection
{
public:
    explicit Connection(ConnectionConfig config, unsigned int maxLostMessages = 3);

    const std::vector<std::uint8_t>& GetInputBuffer() const { return inputBuffer; }
    const std::vector<std::uint8_t>& GetOutputBuffer() const { return outputBuffer; }
    void SetOutputBuffer(std::span<const std::uint8_t> data);
    void ClearInputBuffer() { inputBuffer.clear(); }
    void ClearOutputBuffer() { outputBuffer.clear(); }
    std::size_t GetInputBufferSize() const { return config.inbuffersize; }
    std::size_t GetOutputBufferSize() const { return config.outbuffersize; }

    SocketPerformance GetPerformance() const { return link->GetPerformance(); }
    void ReportLostMessage() { link->ReportLostMessage(); }
    unsigned short LocalPort() const { return link->LocalPort(); }
    ConnectionType GetConnectionType() const { return config.type; }

    bool Connect();
    bool Accept();
    bool Reconnect();
    bool End();
    bool Read();
    bool Write();

private:
    enum class Mode { None, Connect, Accept };

    ConnectionConfig config;
    std::unique_ptr<Link> link;
    std::vector<std::uint8_t> inputBuffer;
    std::vector<std::uint8_t> outputBuffer;
    Mode mode = Mode::None;
};
