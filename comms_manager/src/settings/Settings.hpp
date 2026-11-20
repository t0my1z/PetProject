#pragma once 

#include <memory>
#include <string>
#include <vector>
#include "communication/ConnectionTypes.hpp"

class ConfigurationReader;

struct ConnectionConfig
{
    std::string name;
    ConnectionType type;
    Endianness endianness;
    std::string host;
    int localport;
    int remoteport;
    bool timeout;
    int frequency;
    int inbuffersize;
    int outbuffersize;
};

enum class CommunicationMethod
{
    CHAT
};

struct ModuleConfig
{
    std::string name;
    CommunicationMethod communication_method;
    std::vector<std::string> connections;
};

struct ProgramConfig
{
    std::string config_file_path;
    std::vector<ConnectionConfig> connections;
    std::vector<ModuleConfig> modules;
};

struct ProgramOptions
{
    std::string config_folder;
    std::string module_folder;
};

class Settings
{
public:
    static Settings& GetSettings();

    void InitSettings(int argc, char** argv);

    ProgramConfig& GetProgramConfig();

    const ProgramOptions& GetProgramOptions() const;

    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;

private:
    Settings();

    void ReadProgramOptions(int argc, char** argv);

    ProgramConfig program_config;
    ProgramOptions program_options;

    std::unique_ptr<ConfigurationReader> configuration_reader;
};