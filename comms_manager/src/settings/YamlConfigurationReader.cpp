#include "YamlConfigurationReader.hpp"
#include "Settings.hpp"

#include <filesystem>
#include <print>
#include <iostream>

#include <yaml-cpp/yaml.h>

void YamlConfigurationReader::ReadProgramConfiguration(const std::string& configuration_folder, ProgramConfig& configuration)
{
    // Implementation for reading YAML configuration goes here
    std::filesystem::path config_path = std::filesystem::path(configuration_folder) / "config.yaml";
    if (!std::filesystem::exists(config_path))
    {
        std::println("Configuration file not found: {}", config_path.string());
    }

    configuration.config_file_path = config_path.string();
    YAML::Node node = YAML::LoadFile(configuration.config_file_path);

    if(node["connections"])
    {
        for (const auto& connection_node : node["connections"])
        {
            ConnectionConfig connection;
            connection.name = connection_node["name"].as<std::string>();
            connection.type = connection_node["type"].as<std::string>() == "TCP" ? ConnectionType::TCP : ConnectionType::UDP;
            connection.endianness = connection_node["endianness"].as<std::string>() == "Little" ? Endianness::Little : Endianness::Big;
            connection.host = connection_node["host"].as<std::string>();
            connection.localport = connection_node["localport"].as<int>();
            connection.remoteport = connection_node["remoteport"].as<int>();
            connection.timeout = connection_node["timeout"].as<bool>();
            connection.frequency = connection_node["frequency"].as<int>();
            connection.inbuffersize = connection_node["inbuffersize"].as<int>();
            connection.outbuffersize = connection_node["outbuffersize"].as<int>();

            std::println("Read connection with name '{}' from config file: {}", connection.name, configuration.config_file_path);

            configuration.connections.push_back(connection);
        }
    }
    
    if(node["modules"])
    {
        for (const auto& module_node : node["modules"])
        {
            ModuleConfig module;
            module.name = module_node["name"].as<std::string>();
            module.communication_method = module_node["communication_method"].as<std::string>() == "CHAT" ? CommunicationMethod::CHAT : CommunicationMethod::CHAT;
            if(module_node["connections"])
            {
                for (const auto& connection_name : module_node["connections"])
                {
                    module.connections.push_back(connection_name.as<std::string>());
                }
            }

            std::println("Read module with name '{}' from config file: {}", module.name, configuration.config_file_path);

            configuration.modules.push_back(module);
        }
    }
}