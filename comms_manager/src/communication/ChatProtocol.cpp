#include "communication/ChatProtocol.hpp"
#include "connections/Connection.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace
{
constexpr std::size_t HeaderSize = 8;
constexpr std::size_t CreatedChatRoomPayloadSize = 200 + 100;
constexpr std::size_t JoinChatRoomPayloadSize = 100;
constexpr std::size_t LeaveChatRoomPayloadSize = 100;
constexpr std::size_t SendChatPayloadSize = 200;
constexpr std::size_t MaximumFrameSize = HeaderSize + CreatedChatRoomPayloadSize;

void WriteUint32(std::vector<std::uint8_t>& destination, std::size_t offset,
                 std::uint32_t value)
{
    destination[offset] = static_cast<std::uint8_t>(value >> 24);
    destination[offset + 1] = static_cast<std::uint8_t>(value >> 16);
    destination[offset + 2] = static_cast<std::uint8_t>(value >> 8);
    destination[offset + 3] = static_cast<std::uint8_t>(value);
}

std::uint32_t ReadUint32(const std::vector<std::uint8_t>& source, std::size_t offset)
{
    return (static_cast<std::uint32_t>(source[offset]) << 24) |
           (static_cast<std::uint32_t>(source[offset + 1]) << 16) |
           (static_cast<std::uint32_t>(source[offset + 2]) << 8) |
           static_cast<std::uint32_t>(source[offset + 3]);
}

void WriteString(std::vector<std::uint8_t>& destination, std::size_t& offset,
                 const std::string& value, std::size_t width)
{
    if (value.size() > width || value.find('\0') != std::string::npos)
        throw std::invalid_argument("Chat string does not fit its fixed-width field.");

    std::copy(value.begin(), value.end(), destination.begin() + offset);
    offset += width;
}

std::string ReadString(const std::vector<std::uint8_t>& source, std::size_t& offset,
                       std::size_t width)
{
    const auto first = source.begin() + offset;
    const auto last = first + width;
    const auto terminator = std::find(first, last, std::uint8_t{0});
    if (std::any_of(terminator, last, [](std::uint8_t value) { return value != 0; }))
        throw std::invalid_argument("Chat string field has non-zero data after its terminator.");

    const std::string value(first, terminator);
    offset += width;
    return value;
}

std::size_t PayloadSize(MessageType type)
{
    switch (type)
    {
    case MessageType::CREATED_CHAT_ROOM: return CreatedChatRoomPayloadSize;
    case MessageType::JOIN_CHAT_ROOM: return JoinChatRoomPayloadSize;
    case MessageType::LEAVE_CHAT_ROOM: return LeaveChatRoomPayloadSize;
    case MessageType::SEND_CHAT: return SendChatPayloadSize;
    }
    throw std::invalid_argument("Unknown chat message type.");
}

MessageType GetMessageType(const ChatMessage& message)
{
    return std::visit([](const auto& value) -> MessageType
    {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, CreatedChatRoom>)
            return MessageType::CREATED_CHAT_ROOM;
        else if constexpr (std::is_same_v<T, JoinChatRoom>)
            return MessageType::JOIN_CHAT_ROOM;
        else if constexpr (std::is_same_v<T, LeaveChatRoom>)
            return MessageType::LEAVE_CHAT_ROOM;
        else
            return MessageType::SEND_CHAT;
    }, message);
}

void WritePayload(std::vector<std::uint8_t>& frame, std::size_t& offset,
                  const ChatMessage& message)
{
    std::visit([&](const auto& value)
    {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, CreatedChatRoom>)
        {
            WriteString(frame, offset, value.room_name, 200);
            WriteString(frame, offset, value.admin_name, 100);
        }
        else if constexpr (std::is_same_v<T, JoinChatRoom>)
            WriteString(frame, offset, value.joiner_name, 100);
        else if constexpr (std::is_same_v<T, LeaveChatRoom>)
            WriteString(frame, offset, value.leaver_name, 100);
        else
            WriteString(frame, offset, value.message, 200);
    }, message);
}

ChatMessage ReadPayload(MessageType type, const std::vector<std::uint8_t>& frame)
{
    std::size_t offset = HeaderSize;
    switch (type)
    {
    case MessageType::CREATED_CHAT_ROOM:
        return CreatedChatRoom{ReadString(frame, offset, 200), ReadString(frame, offset, 100)};
    case MessageType::JOIN_CHAT_ROOM:
        return JoinChatRoom{ReadString(frame, offset, 100)};
    case MessageType::LEAVE_CHAT_ROOM:
        return LeaveChatRoom{ReadString(frame, offset, 100)};
    case MessageType::SEND_CHAT:
        return SendChat{ReadString(frame, offset, 200)};
    }
    throw std::invalid_argument("Unknown chat message type.");
}
}

ChatProtocol::ChatProtocol(ConnectionType transport) : transport(transport)
{
    if (transport != ConnectionType::TCP && transport != ConnectionType::UDP)
        throw std::invalid_argument("Unsupported chat transport.");
}

void ChatProtocol::Encode(const void* message, std::vector<std::uint8_t>& output) const
{
    const auto type = GetMessageType(*static_cast<const ChatMessage*>(message));
    const auto frameSize = HeaderSize + PayloadSize(type);
    if (frameSize > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Chat frame exceeds the wire size field.");

    output.resize(frameSize, 0);
    WriteUint32(output, 0, static_cast<std::uint32_t>(frameSize));
    WriteUint32(output, 4, static_cast<std::uint32_t>(type));
    std::size_t offset = HeaderSize;
    WritePayload(output, offset, *static_cast<const ChatMessage*>(message));
}

DecodeResult ChatProtocol::Decode(const std::vector<std::uint8_t>& input, void* message)
{
    if (transport == ConnectionType::UDP)
    {
        std::vector<std::uint8_t> frame(input.begin(), input.end());
        if (frame.size() < HeaderSize)
            throw std::invalid_argument("UDP chat datagram is shorter than its header.");

        const auto frameSize = ReadUint32(frame, 0);
        const auto typeValue = ReadUint32(frame, 4);
        if (frameSize > MaximumFrameSize || frameSize < HeaderSize)
            throw std::invalid_argument("UDP chat datagram has an invalid frame size.");
        const auto type = static_cast<MessageType>(typeValue);
        if (frameSize != HeaderSize + PayloadSize(type) || frame.size() != frameSize)
            throw std::invalid_argument("UDP chat datagram size does not match its message type.");

        *static_cast<ChatMessage*>(message) = ReadPayload(type, frame);
        return DecodeResult::MessageDecoded;
    }

    if (input.size() > std::numeric_limits<std::size_t>::max() - pending.size())
        throw std::length_error("Buffered TCP chat data exceeds the configured input capacity.");
    pending.insert(pending.end(), input.begin(), input.end());

    if (pending.size() < HeaderSize)
        return DecodeResult::NeedMoreData;

    const auto frameSize = ReadUint32(pending, 0);
    const auto typeValue = ReadUint32(pending, 4);
    if (frameSize > MaximumFrameSize || frameSize < HeaderSize)
        throw std::invalid_argument("TCP chat stream has an invalid frame size.");
    const auto type = static_cast<MessageType>(typeValue);
    if (frameSize != HeaderSize + PayloadSize(type))
        throw std::invalid_argument("TCP chat frame size does not match its message type.");
    if (pending.size() < frameSize)
        return DecodeResult::NeedMoreData;

    std::vector<std::uint8_t> frame(pending.begin(), pending.begin() + frameSize);
    *static_cast<ChatMessage*>(message) = ReadPayload(type, frame);
    pending.erase(pending.begin(), pending.begin() + frameSize);
    return DecodeResult::MessageDecoded;
}

void ChatProtocol::Reset()
{
    pending.clear();
}
