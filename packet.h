#pragma once

#include <QtGlobal>

namespace faraz {

inline constexpr quint32 kPacketMagic = 0x46525A31u; // "FRZ1"
inline constexpr quint16 kPacketVersion = 1;
inline constexpr int kMaxPacketBins = 2048;

enum PacketFlags : quint32 {
    PacketFlagNone      = 0u,
    PacketFlagHasCoord  = 1u << 0,
    PacketFlagHasDf     = 1u << 1,
    PacketFlagHasSpec   = 1u << 2,
    PacketFlagDfMode    = 1u << 3,
    PacketFlagResetAxis = 1u << 4,
    PacketFlagResetDf   = 1u << 5
};

struct Q_PACKED Packet
{
    quint32 magic = kPacketMagic;
    quint16 version = kPacketVersion;
    quint16 reserved = 0;
    quint32 flags = PacketFlagNone;

    quint64 timestampMs = 0;

    double longitude = 0.0;
    double latitude = 0.0;

    double centerFrequencyHz = 0.0;
    double spanHz = 0.0;

    double bearingDeg = 0.0;
    double levelDbm = -120.0;

    quint32 binCount = 0;
    float bins[kMaxPacketBins]{};
};

} // namespace faraz