#pragma once

#include <vector>
#include <cstdint>

enum class DecodeResult
{
    MessageDecoded,
    NeedMoreData
};

class Protocol
{
public:
    virtual ~Protocol() = default;
    virtual void Encode(const void* message, std::vector<std::uint8_t>& output) const = 0;
    virtual DecodeResult Decode(const std::vector<std::uint8_t>& input, void* message) = 0;
    virtual void Reset() = 0;
};