#pragma once 

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <cstdint>

#include "communication/ConnectionTypes.hpp"

struct ConnectionConfig;
class Connection;

class ConnectionManager
{
    friend class Connection;
public:
    ConnectionManager() = default;
    ~ConnectionManager();

    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    const std::vector<std::uint8_t>& GetInputBufferFromConnection(const std::string& name) const;
    void SetOutputBufferFromConnection(const std::string& name, const std::vector<std::uint8_t>& data) const;

    void Initialize(const std::vector<ConnectionConfig>& configs);
    void Read();
    void Write();
    void Shutdown();

    void ClearInputBuffers();
    void ClearOutputBuffers();

    ConnectionType GetConnectionType(const std::string& name) const;

private:
    std::map<std::string, std::shared_ptr<Connection>> connections;
};