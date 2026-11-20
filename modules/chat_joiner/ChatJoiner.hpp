#pragma once

#include "module/Module.hpp"
#include "communication/ChatProtocol.hpp"

class ChatJoiner : public Module
{
public:
    ChatJoiner(const std::string& name, CommsManager* commsManager, std::map<std::string,ModuleConnectionData> connectionsData)
        : Module(name, commsManager, std::move(connectionsData))
    {
    }

    ~ChatJoiner() override = default;

    void Initialize() override;
    void Process() override;

private:
    enum class State {
        WAITING_FOR_ROOM,
        ROOM_DISCOVERED,
        ASKING_TO_JOIN,
        CHATTING_WAITING,
        CHATTING_MY_TURN,
        DECLAINED_TO_JOIN,
        EXITING
    };

    State currentState = State::WAITING_FOR_ROOM;
    std::string chatRoomName;
    std::string adminName;
    std::string userName;
};
