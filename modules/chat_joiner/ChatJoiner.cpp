#include "ChatJoiner.hpp"
#include "module/ModuleInterface.hpp"
#include "communication/Protocol.hpp"

#include <iostream>

void ChatJoiner::Initialize()
{
    std::cout << "\n=== Chat Joiner Module ===" << std::endl;
    std::cout << "Waiting for room creation notification..." << std::endl;
    currentState = State::WAITING_FOR_ROOM;
}

void ChatJoiner::Process()
{
    switch (currentState) {
        case State::WAITING_FOR_ROOM:
            {
                // Check for incoming room creation message
                if (!GetInputBuffer("chat_connection").empty()) {
                    ChatMessage receivedMsg;
                    if (GetConnectionDataMap().at("chat_connection").protocol_of_connection->Decode(GetInputBuffer("chat_connection"), &receivedMsg) == DecodeResult::MessageDecoded) {
                        if (auto* createdMsg = std::get_if<CreatedChatRoom>(&receivedMsg)) {
                            chatRoomName = createdMsg->room_name;
                            adminName = createdMsg->admin_name;
                            std::cout << "\nRoom '" << chatRoomName << "' has been created by " << adminName << std::endl;
                            currentState = State::ROOM_DISCOVERED;
                        }
                    }
                }
            }
            break;
            
        case State::ROOM_DISCOVERED:
            {
                std::cout << "Do you want to join this chat room? (yes/no): ";
                std::string response;
                if (std::getline(std::cin, response)) {
                    if (response == "yes" || response == "y") {
                        currentState = State::ASKING_TO_JOIN;
                    } else {
                        currentState = State::DECLAINED_TO_JOIN;
                    }
                }
            }
            break;
            
        case State::ASKING_TO_JOIN:
            {
                std::cout << "Enter your username: ";
                std::getline(std::cin, userName);
                
                // Send join message
                JoinChatRoom joinMsg;
                joinMsg.joiner_name = userName;
                
                ChatMessage msg(joinMsg);
                SendMessage(&msg, "chat_connection");
                
                std::cout << "Sent join request. Waiting for creator's first message..." << std::endl;
                // Joiner waits for creator's first message
                currentState = State::CHATTING_WAITING;
            }
            break;
            
        case State::CHATTING_WAITING:
            {
                // Waiting for creator to send a message
                if (!GetInputBuffer("chat_connection").empty()) {
                    ChatMessage receivedMsg;
                    if (GetConnectionDataMap().at("chat_connection").protocol_of_connection->Decode(GetInputBuffer("chat_connection"), &receivedMsg) == DecodeResult::MessageDecoded) {
                        if (auto* chatMsg = std::get_if<SendChat>(&receivedMsg)) {
                            std::cout << "\n[RECEIVED] " << chatMsg->message << std::endl;
                            // Now it's joiner's turn to respond
                            currentState = State::CHATTING_MY_TURN;
                        }
                    }
                }
            }
            break;
            
        case State::CHATTING_MY_TURN:
            {
                // Joiner's turn to type
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
                        // Now wait for creator's response
                        currentState = State::CHATTING_WAITING;
                    }
                }
            }
            break;
        case State::DECLAINED_TO_JOIN:
            std::cout << "You have declined to join the chat room." << std::endl;
            RequestQuit();
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

MODULE_EXPORT_INTERFACE(ChatJoiner)
