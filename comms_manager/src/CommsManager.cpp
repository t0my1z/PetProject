#include "CommsManager.hpp"

#include "settings/Settings.hpp"
#include "connections/ConnectionManager.hpp"
#include "module/ModuleManager.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

CommsManager::CommsManager()
    : connection_manager(std::make_unique<ConnectionManager>()),
      module_manager(std::make_unique<ModuleManager>())
{
}

CommsManager::~CommsManager()
{
}

void CommsManager::Initialize(int argc, char** argv)
{
    try {
        auto& settings = Settings::GetSettings();
        settings.InitSettings(argc, argv);
        ProgramConfig& config = settings.GetProgramConfig();
        const auto& program_options = settings.GetProgramOptions();

        try {
            connection_manager->Initialize(config.connections);
            std::cout << "Initialized " << config.connections.size() << " connections" << std::endl;
        } 
        catch (const std::exception& e) {
                std::cerr << "Warning: Failed to initialize connections: " << e.what() << std::endl;
                std::cerr << "Continuing with module loading..." << std::endl;
        } 

        fs::path modulesPath;
        if (!program_options.module_folder.empty())
        {
            modulesPath = program_options.module_folder;
        }
        else
        {
            fs::path configFolderPath = fs::path(config.config_file_path).parent_path();
            modulesPath = configFolderPath.parent_path() / "lib" / "modules";
        }
        
        std::cout << "Module path: " << modulesPath << std::endl;
        module_manager->LoadModules(modulesPath.string(), this, connection_manager.get());
        module_manager->InitializeModules();

        std::cout << "\nAll modules initialized and ready" << std::endl;
    } 
    catch (const std::invalid_argument& error) {
        std::cerr << "Error: " << error.what() << '\n'
                  << "Usage: comms_manager --config_folder <path> [--module_folder <path>]\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
    }

    running = true;
}

void CommsManager::Loop()
{
    while (running) {
        try {
            // Read from connections
            connection_manager->Read();
            
            // Process modules
            module_manager->ProcessModules(connection_manager.get());

            // Clear input buffers after processing
            connection_manager->ClearInputBuffers();
            
            // Write to connections
            connection_manager->Write();

            // Clear output buffers after writing
            connection_manager->ClearOutputBuffers();

            if(quit_requested)
                running = false;
            
            // Small delay to prevent CPU spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } 
        catch (const std::exception& e) {
            std::cerr << "Error in event loop: " << e.what() << std::endl;
        }
    }
}

void CommsManager::Shutdown()
{
    module_manager->UnloadModules();
    connection_manager->Shutdown();
}