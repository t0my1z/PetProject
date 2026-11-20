#pragma once

#include "Module.hpp"

#include <memory>

class ConnectionManager;
class Protocol;

typedef Module* (*ModuleFactory)(CommsManager* commsManager, std::map<std::string, ModuleConnectionData>);
typedef void (*ModuleDeleter)(Module* module);

#define MODULE_EXPORT extern "C"

#define MODULE_CREATE_FUNCTION create_module
#define MODULE_DESTROY_FUNCTION destroy_module

#define MODULE_EXPORT_CREATE(ModuleClass) \
    MODULE_EXPORT Module* MODULE_CREATE_FUNCTION(CommsManager* commsManager,std::map<std::string, ModuleConnectionData> connectionsData) \
    { \
        return new ModuleClass(#ModuleClass, commsManager, std::move(connectionsData)); \
    }

#define MODULE_EXPORT_DESTROY \
    MODULE_EXPORT void MODULE_DESTROY_FUNCTION(Module* module) \
    { \
        delete module; \
    }

#define MODULE_EXPORT_INTERFACE(ModuleClass) \
    MODULE_EXPORT_CREATE(ModuleClass) \
    MODULE_EXPORT_DESTROY
