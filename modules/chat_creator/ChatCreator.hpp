#pragma once

#include "module/Module.hpp"
#include "communication/ChatProtocol.hpp"

class ChatCreator : public Module
{
public:
    ChatCreator(const std::string& name, CommsManager* commsManager, std::map<std::string,ModuleConnectionData> connectionsData)
        : Module(name, commsManager, std::move(connectionsData))
    {
    }

    ~ChatCreator() override = default;

    void Initialize() override;
    void Process() override;

private:
    enum class State {
        WAITING_FOR_SETUP,
        ROOM_CREATED,
        WAITING_FOR_JOINER,
        CHATTING_MY_TURN,
        CHATTING_WAITING,
        EXITING
    };

    State currentState = State::WAITING_FOR_SETUP;
    std::string chatRoomName;
    std::string userName;
    std::string joinerName;
};
