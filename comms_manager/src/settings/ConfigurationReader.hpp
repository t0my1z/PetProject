#pragma once

#include <string>

struct ProgramConfig;

class ConfigurationReader
{
public:
    virtual void ReadProgramConfiguration(const std::string& configuration_folder, ProgramConfig& configuration) = 0;
};