# Chat communication protocol

`ICD_CHAT.idl` describes the chat messages; `ChatProtocol.hpp` and
`ChatProtocol.cpp` implement the matching wire encoder and decoder.

Each frame consists of an 8-byte header followed by a fixed-width payload:

* `message_size`: unsigned 32-bit total frame size, including the header.
* `message_type`: unsigned 32-bit value from `MessageType` (`0` through `3`).
* Both header fields use network byte order (big-endian).
* Payload string fields use their declared byte width and are zero-padded.
  Encoders accept at most the field width in bytes and reject embedded NULs.
  Decoders accept a completely full field without a terminator; if a terminator
  exists, all following bytes in that field must be zero.

The resulting frame sizes are 308 bytes for `CreatedChatRoom`, 108 bytes for
`JoinChatRoom` and `LeaveChatRoom`, and 208 bytes for `SendChat`. Configure
connection input and output buffers to accommodate the frame sizes the
application expects to receive and send. A UDP datagram carries exactly one
complete frame; therefore its input buffer must fit the largest datagram used.

Create `ChatProtocol` with the connection's transport type. `Encode` writes one
frame into a `Connection` output buffer; call `Connection::Write` to send it.
After a successful `Connection::Read`, call `Decode`. Decoding consumes the
connection input buffer. For TCP, the decoder retains partial frames and any
additional complete frames internally. When it returns `NeedMoreData`, read
again. After it returns `MessageDecoded`, call `Decode` again to drain any
already-buffered TCP frames before reading more. `Reset` discards buffered TCP
data. UDP decoding is performed once per successful datagram read; malformed
or empty datagrams throw `std::invalid_argument`.

The format is fixed-width and byte-oriented. It does not define text encoding,
authentication, encryption, delivery guarantees, or application-level message
acknowledgements. Message loss detection and retransmission remain outside this
codec.
