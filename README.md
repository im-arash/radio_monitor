<h1>🛜 Radio Monitor</h1>
<img width="1276" height="753" alt="image" src="https://github.com/user-attachments/assets/df9724a7-d691-4b1a-953f-5baad889991e" />

## Demo: Multiple Stations
https://github.com/user-attachments/assets/08f4b7ea-1cda-4765-9ccf-f8751c5c220a

## Demo: Tuning
https://github.com/user-attachments/assets/3713d2b6-4823-44a0-9468-f7cb4ce81b17



## The Work Flow

For testing purposes, system is decoupled into two separate applications:

- **Station Simulator:** streams binary packets (geolocation + frequency + ...) via TCP to the Radio Monitor app → Transmitter
- **Control Center:** our main Radio-Monitor application. For now control center works as visualization (Spectrum Analyzer, Waterfall Display & Heatmap)

<img width="2354" height="2961" alt="mermaid-diagram (12)" src="https://github.com/user-attachments/assets/ec873f04-af5a-4a45-a590-ff892b960c15" />

## Getting Started

**Requirements**

- Qt 6.10+ (Quick, Location, Positioning)
- CMake 3.16+
- A C++23 compiler
- Internet access for the map tiles

**Run order matters**, Control Center is the server so it must start first:

1. Build & run **Control Center**. It creates `Station 1` and listens on port `9000`.
2. Build & run **Station Simulator**, set `port` to `9000` and click *Streaming*.
3. Hit **+ New** in Control Center to add another station, it listens on `9001`, then `9002` and so on (`basePort + station index`).

## How it works

1. Run control center as server, creating new stations initiate servers with unique ports.

   *each port accepts one client at a time. If a new client connects, the old one gets dropped.*

---

2. Run station simulator as client, each client streams data over TCP.

   *both apps use a shared binary data structure `packet.h` which is more performant and avoids serialization/deserialization overhead. Since there is no physical transmitter we basically stream noise + geolocation, frequency, bearing line degree, power level and ... .*

   *the simulator also fakes the RF side: it calculates the distance between receiver and transmitter, derives the received power (dBm) and checks it against the detection threshold. Bearing noise grows as SNR drops, with rare multipath outliers, so the heatmap behaves like a real DF would.*

<img width="800" height="566" alt="image" src="https://github.com/user-attachments/assets/56c91949-a48c-4c23-81ce-277b8de3e3b1" />

---

3. Communication between two apps is bidirectional, which means you can change the parameters in control center.

   *aside from the frequency section in control center (bottom panel), other options are not implemented yet.*

---

4. The continuous stream updates Items (Spectrum Analyzer, Waterfall and Heatmap) in control center.

   *Items are based on QPainter & QQuickPaintedItem for CPU processing support, so the app can run on lower-end devices.*

<!-- <img width="5122" height="722" alt="mermaid-diagram (10)" src="https://github.com/user-attachments/assets/551aa6aa-6aac-4cc0-bdd8-0650d597287c" /> -->

## Protocol

### Station → Control Center (binary)

Every frame looks like this:

```
[ 4 bytes: payload size, big-endian ][ payload: Packet header + binCount * float ]
```

- Payload is the raw `faraz::Packet` struct from `packet.h`.
- Header is 72 bytes, then `binCount` floats (dBm values). Frames are variable-length, so you don't have to send all 2048 bins.
- Limits: `kMaxPacketBins = 2048`, max frame size 1 MB. Anything bigger clears the buffer.
- Frames with wrong `magic` (`0x46525A31`, "FRZ1") or `version` (`1`) are dropped.

| Field | Type | Meaning |
|---|---|---|
| `magic` / `version` / `reserved` | u32 / u16 / u16 | frame validation |
| `flags` | u32 | which parts of the packet are valid (below) |
| `timestampMs` | u64 | timestamp |
| `longitude`, `latitude` | double | station location |
| `centerFrequencyHz`, `spanHz` | double | current tuning |
| `bearingDeg`, `levelDbm` | double | DF measurement |
| `binCount`, `bins[]` | u32, float[] | spectrum |

| Flag | Bit | Meaning |
|---|---|---|
| `PacketFlagHasCoord` | 0 | lon/lat are valid |
| `PacketFlagHasDf` | 1 | bearing + level are valid → heatmap |
| `PacketFlagHasSpec` | 2 | bins are valid → spectrum & waterfall |
| `PacketFlagDfMode` | 3 | station is in DF mode |
| `PacketFlagResetAxis` | 4 | visualizers should reset their axis |
| `PacketFlagResetDf` | 5 | clear the heatmap |

### Control Center → Station (JSON, one message per line)

The control direction is not binary, it's low-rate so plain JSON is fine:

```json
{"type":"tune","centerFrequencyHz":446000000,"spanHz":200000,"dfMode":true}
```

Each message ends with `\n`.

> `packet.h` is duplicated in both projects (not an actual shared file).

## Architectural Decisions
How the pieces fit together (minimal, not every class):
<img width="2462" height="2148" alt="class-diagram" src="https://github.com/user-attachments/assets/92f2a83d-91a1-45e8-849b-68a3b4bdd566" />

 
- **MVVM / Model-view ✅** related to StationState & StationModel.
- **Singleton ✅** Theme is a global state and it shouldn't be initiated more than once (it's a QML singleton).
- **Interface (ILink) ✅** `StationModel` talks to `ILink`, rather than talking to specific protocol.
- **Factory ⚠️** for now new stations are managed by StationState & StationModel because most requirements are not revealed yet, later on if each station has different behavior Factory pattern could be the right candidate to implement (`createLink()` is already the single place where it would go).
### SOLID
 
I tried to apply SOLID as much as possible for this scale without overengineering.
 
- **Single responsibility:** each class has one reason to change.
  - `TcpLink`: sockets and framing, doesn't know what a station is.
  - `StationState`: one station's data, applies packets.
  - `StationModel`: the list of stations and wiring.
  - Each painted item (SpectrumWaveItem, WaterfallItem, HeatmapItem) draws one thing.
- **Open/closed:** 
  - New visualizers plug into `StationState` signals (`frameReady`, `bearingMeasured`), so the existing items and `StationState` itself don't change. You only add the new item and its QML wiring.
  - A new protocol is a new `ILink` implementation, plus one line in `StationModel::createLink()`.
- **Dependency inversion:** `StationModel` depends on `ILink`, not on `TcpLink`. Visualizers depend on `StationState` signals, `StationState` doesn't know visualizers exist.
- **Liskov / Interface segregation:** not much to apply at this size. `ILink` has one implementation and three methods, so there's nothing to violate yet. I didn't force extra interfaces just to tick the box.

---

## Future Implementations/Improvements

- **Recorder / Database:** adding history based on timestamps.
- **Threat Notifier:** we should be able to define our own pattern, and when that pattern is visible to the app, it counts it as a threat and notifies us.

  *for example we could define a pattern for half-duplex communications within the UHF band (446.0 MHz) that can be flagged as a threat.*

  *the "Threat Detected" label in the top bar is only a placeholder for now, it's always visible and not connected to anything.*

- **More links:** a second `ILink` implementation (UDP, serial or recorded file replay).

## Features

Theme Color Selection with presets

<img width="1273" height="755" alt="image" src="https://github.com/user-attachments/assets/74f14768-9fe6-4234-a494-75ddff224848" />
<img width="1272" height="754" alt="image" src="https://github.com/user-attachments/assets/523a4c22-8e20-496f-bc29-d80b637e979f" />


