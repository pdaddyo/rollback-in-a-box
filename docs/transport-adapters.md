# Transport Adapters

The core addon is transport-agnostic. A transport adapter only sends and
receives opaque `PackedByteArray` packets.

## Direct UDP

Use `PacketPeerUDP` to send `session.get_packet()` to a peer address and pass
received packets into `session.ingest_packet(packet)`.

## Steam, ENet, WebRTC, Relay

Use the same contract:

- reliable delivery is not required
- ordered delivery is not required
- duplicated packets are fine
- dropped packets are fine
- packet contents are opaque

The rollback packet contains redundant input history and ack data, so unreliable
transports work well.

Matchmaking data such as player index, seed, map, and content version should be
exchanged before `session.start()`. It is not part of the rollback packet.
