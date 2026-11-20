#pragma once

#include <memory>

class ModuleManager;
class ConnectionManager;

class CommsManager
{
    
public:
    CommsManager();
    ~CommsManager();

    CommsManager(const CommsManager&) = delete;
    CommsManager& operator=(const CommsManager&) = delete;

    void Initialize(int argc, char** argv);
    void Loop();
    void Shutdown();

    void RequestQuit() { quit_requested = true; }

private:
    std::unique_ptr<ConnectionManager> connection_manager;
    std::unique_ptr<ModuleManager> module_manager;
    bool running = false;
    bool quit_requested = false;
};