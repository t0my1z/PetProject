#pragma once

#include "communication/Protocol.hpp"
#include "CommsManager.hpp"

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <map>


struct ModuleConnectionData
{
    std::unique_ptr<Protocol> protocol_of_connection;
    std::vector<std::uint8_t> input_buffer;
    std::vector<std::uint8_t> output_buffer;
};

class Module
{
public:
    Module(const std::string& name, CommsManager* commsManager, std::map<std::string,ModuleConnectionData> connectionsData) 
    : name(name), comms_manager(commsManager)
    {
        connections = std::move(connectionsData);
    }

    virtual ~Module() = default;
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    virtual void Initialize() = 0;
    virtual void Process() = 0;

    const std::string& GetName() const { return name; }
    
    void ReceiveMessage(const std::vector<std::uint8_t>& message, const std::string& connectionName)
    {
        connections.at(connectionName).input_buffer = message;  // Store raw encoded bytes
    }

    void SendMessage(const void* message, const std::string& connectionName)
    {
        connections.at(connectionName).output_buffer.clear();
        connections.at(connectionName).protocol_of_connection->Encode(message, connections.at(connectionName).output_buffer);
    }
    
    const std::vector<std::uint8_t>& GetOutputBuffer(const std::string& connectionName) const { return connections.at(connectionName).output_buffer; }
    const std::vector<std::uint8_t>& GetInputBuffer(const std::string& connectionName) const { return connections.at(connectionName).input_buffer; }
    void ClearInputBuffer(const std::string& connectionName) { connections.at(connectionName).input_buffer.clear(); }
    void ClearOutputBuffer(const std::string& connectionName) { connections.at(connectionName).output_buffer.clear(); }

    const std::map<std::string, ModuleConnectionData>& GetConnectionDataMap() const { return connections; }

    void RequestQuit() { if(comms_manager) comms_manager->RequestQuit(); }

private:
    std::string name;
    CommsManager* comms_manager = nullptr;
    std::map<std::string, ModuleConnectionData> connections;
};