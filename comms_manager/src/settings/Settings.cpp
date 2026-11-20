#include "Settings.hpp"
#include "YamlConfigurationReader.hpp"

#include <stdexcept>
#include <string_view>
#include <utility>

Settings& Settings::GetSettings()
{
    static Settings settings;
    return settings;
}

Settings::Settings()
{

}

void Settings::InitSettings(int argc, char** argv)
{
    ReadProgramOptions(argc, argv);

    configuration_reader = std::make_unique<YamlConfigurationReader>();
    configuration_reader->ReadProgramConfiguration(program_options.config_folder, program_config);
}

void Settings::ReadProgramOptions(int argc, char** argv)
{
    std::string config_folder;
    std::string module_folder;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument = argv[i];
        if (argument == "--config_folder")
        {
            if (!config_folder.empty())
            {
                throw std::invalid_argument("--config_folder must only be specified once.");
            }

            if (i + 1 >= argc || std::string_view(argv[i + 1]).empty()
                || std::string_view(argv[i + 1]).starts_with("-"))
            {
                throw std::invalid_argument("--config_folder requires a non-empty path.");
            }

            config_folder = argv[++i];
        }
        else if (argument == "--module_folder")
        {
            if (!module_folder.empty())
            {
                throw std::invalid_argument("--module_folder must only be specified once.");
            }

            if (i + 1 >= argc || std::string_view(argv[i + 1]).empty()
                || std::string_view(argv[i + 1]).starts_with("-"))
            {
                throw std::invalid_argument("--module_folder requires a non-empty path.");
            }

            module_folder = argv[++i];
        }
        else
        {
            throw std::invalid_argument("Unknown argument: " + std::string(argument));
        }
    }

    if (config_folder.empty())
    {
        throw std::invalid_argument("Required argument missing: --config_folder <path>.");
    }

    program_options.config_folder = std::move(config_folder);
    program_options.module_folder = std::move(module_folder);
}

ProgramConfig& Settings::GetProgramConfig()
{
    return program_config;
}

const ProgramOptions& Settings::GetProgramOptions() const
{
    return program_options;
}