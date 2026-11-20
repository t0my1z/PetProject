#include "module/ModuleManager.hpp"
#include "module/ModuleInterface.hpp"
#include "CommsManager.hpp"
#include "connections/ConnectionManager.hpp"
#include "communication/Protocol.hpp"
#include "communication/ChatProtocol.hpp"
#include "settings/Settings.hpp"

#include <iostream>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

ModuleManager::~ModuleManager()
{
    UnloadModules();
}

void ModuleManager::LoadModules(const std::string& modulesPath, CommsManager* commsManager, ConnectionManager* connectionManager)
{
    if (!fs::exists(modulesPath))
    {
        std::cerr << "Modules path does not exist: " << modulesPath << std::endl;
        return;
    }

    std::cout << "Loading modules from: " << modulesPath << std::endl;

    const auto& config = Settings::GetSettings().GetProgramConfig();

    try
    {
        // Load only modules that are configured
        for (const auto& moduleConfig : config.modules)
        {
            std::string moduleName = moduleConfig.name;
            std::cout << "Loading configured module: " << moduleName << std::endl;

            fs::path moduleDir = fs::path(modulesPath) / moduleName;

            if (!fs::exists(moduleDir))
            {
                std::cerr << "Module directory not found: " << moduleDir << std::endl;
                continue;
            }

            if (!fs::is_directory(moduleDir))
            {
                std::cerr << "Module path is not a directory: " << moduleDir << std::endl;
                continue;
            }

            // Look for the shared library in the module directory
            // Convention: lib{moduleName}.so (Linux) or {moduleName}.dll (Windows)
#ifdef _WIN32
            std::string libName = moduleName + ".dll";
#else
            std::string libName = "lib" + moduleName + ".so";
#endif

            fs::path libPath = moduleDir / libName;

            if (!fs::exists(libPath))
            {
                std::cerr << "Module library not found: " << libPath << std::endl;
                continue;
            }

            std::cout << "Loading module library: " << libPath << std::endl;

            // Load the shared library
            void* handle = nullptr;
#ifdef _WIN32
            handle = (void*)LoadLibraryW(libPath.wstring().c_str());
            if (!handle)
            {
                DWORD error = GetLastError();
                std::cerr << "Failed to load module " << moduleName << ": error code " << error << std::endl;
                continue;
            }
#else
            handle = dlopen(libPath.string().c_str(), RTLD_LAZY);
            if (!handle)
            {
                std::cerr << "Failed to load module " << moduleName << ": " << dlerror() << std::endl;
                continue;
            }
#endif

            // Get the factory function
            ModuleFactory factory = nullptr;
#ifdef _WIN32
            factory = (ModuleFactory)GetProcAddress((HMODULE)handle, "create_module");
            if (!factory)
            {
                DWORD error = GetLastError();
                std::cerr << "Failed to get create_module function from " << moduleName 
                          << " (error code: " << error << ")" << std::endl;
                FreeLibrary((HMODULE)handle);
                continue;
            }
#else
            factory = reinterpret_cast<ModuleFactory>(dlsym(handle, "create_module"));
            if (!factory)
            {
                std::cerr << "Failed to get create_module function from " << moduleName << ": " << dlerror() << std::endl;
                dlclose(handle);
                continue;
            }
#endif

            // Create the module instance
            // For now, we'll use ChatProtocol as the default; this could be made configurable
            std::map<std::string, ModuleConnectionData> connectionDataMap;

            for(auto& connection : moduleConfig.connections)
            {
                connectionDataMap[connection] = ModuleConnectionData{};
                ConnectionType type = connectionManager->GetConnectionType(connection);
                std::unique_ptr<Protocol> protocol = nullptr;
                switch(moduleConfig.communication_method)
                {
                    case CommunicationMethod::CHAT:
                        protocol = std::make_unique<ChatProtocol>(type);
                        break;
                    default:
                        std::cerr << "Unsupported communication method" << std::endl;
                        continue;
                }
                connectionDataMap[connection].protocol_of_connection = std::move(protocol);
            }

            std::unique_ptr<Module> module(factory(commsManager, std::move(connectionDataMap)));

            if (!module)
            {
                std::cerr << "Failed to create module instance: " << moduleName << std::endl;
#ifdef _WIN32
                FreeLibrary((HMODULE)handle);
#else
                dlclose(handle);
#endif
                continue;
            }

            std::cout << "Successfully loaded module: " << moduleName << std::endl;

            LoadedModule loadedModule;
            loadedModule.handle = handle;
            loadedModule.module = std::move(module);
            loadedModules.push_back(std::move(loadedModule));
            moduleHandles[moduleName] = handle;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading modules: " << e.what() << std::endl;
    }
}

void ModuleManager::InitializeModules()
{
    std::cout << "Initializing " << loadedModules.size() << " modules" << std::endl;
    for (auto& loadedModule : loadedModules)
    {
        try
        {
            std::cout << "Initializing module: " << loadedModule.module->GetName() << std::endl;
            loadedModule.module->Initialize();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to initialize module: " << e.what() << std::endl;
        }
    }
}

void ModuleManager::ProcessModules(ConnectionManager* connectionManager)
{
    for (auto& loadedModule : loadedModules)
    {
        for(auto& [connectionName, connectionData] : loadedModule.module->GetConnectionDataMap())
        {
            loadedModule.module->ReceiveMessage(connectionManager->GetInputBufferFromConnection(connectionName), connectionName);
        }
        try
        {
            loadedModule.module->Process();
            for(auto& [connectionName, connectionData] : loadedModule.module->GetConnectionDataMap())
                loadedModule.module->ClearInputBuffer(connectionName);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error processing module " << loadedModule.module->GetName() << ": " << e.what() << std::endl;
        }

        for(auto& [connectionName, connectionData] : loadedModule.module->GetConnectionDataMap())
        {
            if(loadedModule.module->GetOutputBuffer(connectionName).size() > 0)
                connectionManager->SetOutputBufferFromConnection(connectionName, loadedModule.module->GetOutputBuffer(connectionName));

            // Clear output buffer after sending
            loadedModule.module->ClearOutputBuffer(connectionName);
        }
    }
}

void ModuleManager::UnloadModules()
{
    std::cout << "Unloading modules" << std::endl;
    for (auto& loadedModule : loadedModules)
    {
        if (loadedModule.handle)
        {
#ifdef _WIN32
            FreeLibrary((HMODULE)loadedModule.handle);
#else
            dlclose(loadedModule.handle);
#endif
        }
    }
    loadedModules.clear();
    moduleHandles.clear();
}
