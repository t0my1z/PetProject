#include "module/ModuleManager.hpp"
#include "module/ModuleInterface.hpp"
#include "CommsManager.hpp"
#include "connections/ConnectionManager.hpp"
#include "communication/Protocol.hpp"
#include "communication/ChatProtocol.hpp"
#include "settings/Settings.hpp"
#include "Log.hpp"

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

    log_info("Loading modules from: {}", modulesPath);

    const auto& config = Settings::GetSettings().GetProgramConfig();

    try
    {
        // Load only modules that are configured
        for (const auto& moduleConfig : config.modules)
        {
            std::string moduleName = moduleConfig.name;
            log_info("Loading configured module: {}", moduleName);

            fs::path moduleDir = fs::path(modulesPath) / moduleName;

            if (!fs::exists(moduleDir))
            {
                log_error("Module directory not found: {}", moduleDir.string());
                continue;
            }

            if (!fs::is_directory(moduleDir))
            {
                log_error("Module path is not a directory: {}", moduleDir.string());
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
                log_error("Module library not found: {}", libPath.string());
                continue;
            }

            log_info("Loading module library: {}", libPath.string());

            // Load the shared library
            void* handle = nullptr;
#ifdef _WIN32
            handle = (void*)LoadLibraryW(libPath.wstring().c_str());
            if (!handle)
            {
                DWORD error = GetLastError();
                log_error("Failed to load module {}: error code {}", moduleName, error);
                continue;
            }
#else
            handle = dlopen(libPath.string().c_str(), RTLD_LAZY);
            if (!handle)
            {
                log_error("Failed to load module {}: {}", moduleName, dlerror());
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
                log_error("Failed to get create_module function from {}: error code {}", moduleName, error);
                FreeLibrary((HMODULE)handle);
                continue;
            }
#else
            factory = reinterpret_cast<ModuleFactory>(dlsym(handle, "create_module"));
            if (!factory)
            {
                log_error("Failed to get create_module function from {}: {}", moduleName, dlerror());
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
                        log_error("Unsupported communication method");
                        continue;
                }
                connectionDataMap[connection].protocol_of_connection = std::move(protocol);
            }

            std::unique_ptr<Module> module(factory(commsManager, std::move(connectionDataMap)));

            if (!module)
            {
                log_error("Failed to create module instance: {}", moduleName);
#ifdef _WIN32
                FreeLibrary((HMODULE)handle);
#else
                dlclose(handle);
#endif
                continue;
            }

            log_info("Successfully loaded module: {}", moduleName);

            LoadedModule loadedModule;
            loadedModule.handle = handle;
            loadedModule.module = std::move(module);
            loadedModules.push_back(std::move(loadedModule));
            moduleHandles[moduleName] = handle;
        }
    }
    catch (const std::exception& e)
    {
        log_error("Error loading modules: {}", e.what());
    }
}

void ModuleManager::InitializeModules()
{
    log_info("Initializing {} modules", loadedModules.size());
    for (auto& loadedModule : loadedModules)
    {
        try
        {
            log_info("Initializing module: {}", loadedModule.module->GetName());
            loadedModule.module->Initialize();
        }
        catch (const std::exception& e)
        {
            log_error("Failed to initialize module {}: {}", loadedModule.module->GetName(), e.what());
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
    log_info("Unloading {} modules", loadedModules.size());
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
