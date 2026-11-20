#include "connections/ConnectionManager.hpp"
#include "connections/Connection.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <print>

namespace
{
std::string ConnectionError(const std::string& operation, const std::string& name,
                            const Connection& connection)
{
    const auto performance = connection.GetPerformance();
    return "Failed to " + operation + " connection '" + name + "' (status " +
           std::to_string(performance.status) + ", system error " +
           std::to_string(performance.sysError) + ").";
}
}

ConnectionManager::~ConnectionManager()
{
    for (const auto& [name, connection] : connections)
    {
        if (!connection->End())
            std::cerr << ConnectionError("close", name, *connection) << '\n';
    }
}

const std::vector<std::uint8_t>& ConnectionManager::GetInputBufferFromConnection(const std::string& name) const
{
    const auto found = connections.find(name);
    if (found == connections.end())
        throw std::out_of_range("Unknown connection: '" + name + "'.");
    return found->second->GetInputBuffer();
}

void ConnectionManager::SetOutputBufferFromConnection(const std::string& name, const std::vector<std::uint8_t>& data) const
{
    const auto found = connections.find(name);
    if (found == connections.end())
        std::cerr << "Didnt find connection: " << name << '\n';
    found->second->SetOutputBuffer(data);
}

void ConnectionManager::Initialize(const std::vector<ConnectionConfig>& configs)
{
    std::map<std::string, std::shared_ptr<Connection>> newConnections;
    for (const auto& config : configs)
    {
        if (config.name.empty())
            throw std::invalid_argument("Connection names must not be empty.");
        if (newConnections.contains(config.name))
            throw std::invalid_argument("Duplicate connection name: '" + config.name + "'.");
        newConnections.emplace(config.name, std::make_shared<Connection>(config));
    }

    Shutdown();
    for (const auto& config : configs)
    {
        const auto& connection = newConnections.at(config.name);
        if (!connection->Connect())
            throw std::runtime_error(ConnectionError("connect", config.name, *connection));
    }
    connections = std::move(newConnections);
}

void ConnectionManager::Read()
{
    for (const auto& [name, connection] : connections)
    {
        if (!connection->Read())
            throw std::runtime_error(ConnectionError("read", name, *connection));
    }
}

void ConnectionManager::Write()
{
    for (const auto& [name, connection] : connections)
    {
        if (!connection->GetOutputBuffer().empty() && !connection->Write())
            throw std::runtime_error(ConnectionError("write", name, *connection));
    }
}

void ConnectionManager::Shutdown()
{
    std::string errors;
    for (const auto& [name, connection] : connections)
    {
        if (!connection->End())
        {
            if (!errors.empty())
                errors += '\n';
            errors += ConnectionError("close", name, *connection);
        }
    }
    connections.clear();
    if (!errors.empty())
        throw std::runtime_error(errors);
}

void ConnectionManager::ClearInputBuffers()
{
    for (const auto& [name, connection] : connections)
    {
        connection->ClearInputBuffer();
    }
}

void ConnectionManager::ClearOutputBuffers()
{
    for (const auto& [name, connection] : connections)
    {
        connection->ClearOutputBuffer();
    }
}

ConnectionType ConnectionManager::GetConnectionType(const std::string& name) const
{
    const auto found = connections.find(name);
    if (found == connections.end())
        throw std::invalid_argument("Did not find connection: " + name);
    return found->second->GetConnectionType();
}