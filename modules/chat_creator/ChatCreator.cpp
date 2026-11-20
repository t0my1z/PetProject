#include "ChatCreator.hpp"
#include "module/ModuleInterface.hpp"
#include "communication/Protocol.hpp"

#include <iostream>

void ChatCreator::Initialize()
{
    std::cout << "\n=== Chat Creator Module ===" << std::endl;
    currentState = State::WAITING_FOR_SETUP;
}

void ChatCreator::Process()
{
    switch (currentState) {
        case State::WAITING_FOR_SETUP:
            {
                std::cout << "Enter chat room name: ";
                std::getline(std::cin, chatRoomName);
                
                std::cout << "Enter your username: ";
                std::getline(std::cin, userName);
                
                // Create and send the room creation message
                CreatedChatRoom createMsg;
                createMsg.room_name = chatRoomName;
                createMsg.admin_name = userName;
                
                ChatMessage msg(createMsg);
                SendMessage(&msg, "chat_connection");
                
                std::cout << "Chat room '" << chatRoomName << "' created by " << userName << std::endl;
                std::cout << "Waiting for joiner..." << std::endl;
                
                currentState = State::ROOM_CREATED;
            }
            break;
            
        case State::ROOM_CREATED:
            {
                // Check for incoming join message
                if (!GetInputBuffer("chat_connection").empty()) {
                    ChatMessage receivedMsg;
                    if (GetConnectionDataMap().at("chat_connection").protocol_of_connection->Decode(GetInputBuffer("chat_connection"), &receivedMsg) == DecodeResult::MessageDecoded) {
                        if (auto* joinMsg = std::get_if<JoinChatRoom>(&receivedMsg))
                        {
                            joinerName = joinMsg->joiner_name;
                            std::cout << "\n" << joinerName << " has joined the chat!" << std::endl;
                            std::cout << "You can now type the first message..." << std::endl;
                            currentState = State::CHATTING_MY_TURN; 
                        }
                    }
                }
            }
            break;
            
        case State::CHATTING_MY_TURN:
            {
                std::cout << "Enter a message (or 'exit' to quit): ";
                std::string input;
                if (std::getline(std::cin, input)) {
                    if (input == "exit") {
                        currentState = State::EXITING;
                        return;
                    }
                    
                    if (!input.empty()) {
                        SendChat sendMsg;
                        sendMsg.message = userName + ": " + input;
                        ChatMessage msg(sendMsg);
                        SendMessage(&msg, "chat_connection");
                        std::cout << "[SENT] " << sendMsg.message << std::endl;
                        currentState = State::CHATTING_WAITING;
                    }
                }
            }
            break;
            
        case State::CHATTING_WAITING:
            {
                // Waiting for joiner to send a message
                if (!GetInputBuffer("chat_connection").empty()) {
                    ChatMessage receivedMsg;
                    if (GetConnectionDataMap().at("chat_connection").protocol_of_connection->Decode(GetInputBuffer("chat_connection"), &receivedMsg) == DecodeResult::MessageDecoded) {
                        if (auto* chatMsg = std::get_if<SendChat>(&receivedMsg))
                        {
                            std::cout << "\n[RECEIVED] " << chatMsg->message << std::endl;
                            currentState = State::CHATTING_MY_TURN;
                        }
                    }
                }
            }
            break;
            
        case State::EXITING:
            SendChat sendMsg;
            sendMsg.message = userName + ": " + "left the chat room";
            ChatMessage msg(sendMsg);
            SendMessage(&msg, "chat_connection");
            RequestQuit();
            break;
    }
}

MODULE_EXPORT_INTERFACE(ChatCreator)
