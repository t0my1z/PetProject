#pragma once

#include "Protocol.hpp"
#include "ConnectionTypes.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <memory>



enum class MessageType : std::uint32_t
{
    CREATED_CHAT_ROOM = 0,
    JOIN_CHAT_ROOM = 1,
    LEAVE_CHAT_ROOM = 2,
    SEND_CHAT = 3
};

struct CreatedChatRoom
{
    std::string room_name;
    std::string admin_name;
    bool operator==(const CreatedChatRoom&) const = default;
};

struct JoinChatRoom
{
    std::string joiner_name;
    bool operator==(const JoinChatRoom&) const = default;
};

struct LeaveChatRoom
{
    std::string leaver_name;
    bool operator==(const LeaveChatRoom&) const = default;
};

struct SendChat
{
    std::string message;
    bool operator==(const SendChat&) const = default;
};

using ChatMessage = std::variant<CreatedChatRoom, JoinChatRoom, LeaveChatRoom, SendChat>;

class ChatProtocol : public Protocol
{
public:
    explicit ChatProtocol(ConnectionType transport);

    void Encode(const void* message, std::vector<std::uint8_t>& output) const override;
    DecodeResult Decode(const std::vector<std::uint8_t>& input, void* message) override;
    void Reset() override;

private:
    ConnectionType transport;
    std::vector<std::uint8_t> pending;
};

