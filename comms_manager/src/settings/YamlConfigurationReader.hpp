#pragma once

#include "ConfigurationReader.hpp"

class YamlConfigurationReader : public ConfigurationReader
{
public:
    virtual void ReadProgramConfiguration(const std::string& configuration_folder, ProgramConfig& configuration) override;
};