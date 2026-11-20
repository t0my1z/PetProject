#pragma once

#include "module/Module.hpp"

#include <memory>
#include <string>
#include <vector>
#include <map>

class ConnectionManager;
class CommsManager;
class Protocol;

class ModuleManager
{
public:
    ModuleManager() = default;
    ~ModuleManager();

    ModuleManager(const ModuleManager&) = delete;
    ModuleManager& operator=(const ModuleManager&) = delete;

    void LoadModules(const std::string& modulesPath, CommsManager* commsManager, ConnectionManager* connectionManager);
    void InitializeModules();
    void ProcessModules(ConnectionManager* connectionManager);
    void UnloadModules();

private:
    struct LoadedModule
    {
        void* handle;
        std::unique_ptr<Module> module;
    };

    std::vector<LoadedModule> loadedModules;
    std::map<std::string, void*> moduleHandles;
};
