# uBridge

uBridge is a lightweight, modular C++ service that detects and manages uThing devices, streams their measurements over NNG PUB/SUB, and exposes control and status via NNG REQ/REP. It’s designed to run efficiently on Linux gateways and SBCs.

![uBridge](/img/ubridge-diagram.svg)

Official documentation: https://docs.ohmtech.io/uBridge/intro/

## Features
- Device auto-discovery of connected uThing devices (USB serial)
- PUB/SUB data stream on a configurable NNG socket
- REQ/REP control API for config, device listing, queries, commands, and statistics
- Systemd units for easy install/run on Linux
- Low resource usage

## Architecture overview
- Bridge process (`ubridge`) starts two sockets:
  - REQ/REP server for control: default `ipc:///tmp/ubridgeReqResp`
  - PUB stream for data: default `ipc:///tmp/ubridgeStream`
- Device monitor scans `/dev/ttyACM*` and `/dev/ttyUSB*` (or a custom base) and spawns per-device relay threads.
- Published topics: `/sensors/<channelID>` with JSON payloads.
- Client example: `consumer` demonstrates REQ/REP calls and PUB subscriptions.

## Command line
```bash
ubridge -c <config.json> -v <verbosity>
```
- `-c, --config` JSON configuration path (default: `/etc/ubridge/ubridgeConfig.json`)
- `-v, --verbose` log verbosity 0–9 (forwarded to loguru)

Logs are written to `/tmp/ubridge.log`.

## Configuration
File: `/etc/ubridge/ubridgeConfig.json` (installed via `make install`). Comments are allowed and ignored.
```json
{
    "devNameBase": "",
    "configSockUrl": "ipc:///tmp/ubridgeReqResp",
    "streamSockUrl": "ipc:///tmp/ubridgeStream",
    "maxDevices": 10
}
```
- `devNameBase`: serial device prefix. If empty, scans `/dev/ttyACM*` and `/dev/ttyUSB*`. On macOS use `/dev/cu`.
- `configSockUrl`: NNG address for the REQ/REP config server. Use `ipc://` for local, or `tcp://host:port` for remote clients.
- `streamSockUrl`: NNG address for the PUB stream. Same IPC vs TCP guidance applies.
- `maxDevices`: maximum number of devices managed.

## Build prerequisites (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y git cmake ninja-build libserial-dev
```

### NNG (recommended pinned commit)
```bash
cd /tmp
git clone https://github.com/nanomsg/nng.git
cd nng
git checkout 9d6b241
mkdir build && cd build
cmake -G Ninja ..
ninja
sudo ninja install
```

## Build
```bash
git clone https://github.com/ohmtech-io/uBridge.git
cd uBridge
mkdir build && cd build
cmake ..
make -j
```

This builds two binaries:
- `ubridge`: the bridge service
- `consumer`: sample client

## Install and run (systemd)
```bash
sudo make install
sudo systemctl enable ubridge ubridge-server
sudo systemctl start ubridge
```

Check status and logs:
```bash
systemctl status ubridge-server.service
tail -f /tmp/ubridge.log
```

## Client/API examples
- List devices (NNG REQ/REP):
```bash
nngcat --req --dial ipc:///tmp/ubridgeReqResp --data '{"request":"getDevices"}'
```
- Query a device by channel ID:
```bash
nngcat --req --dial ipc:///tmp/ubridgeReqResp --data '{"request":"queryDevice","channelID":"uThing::VOC_1234","query":{"status":true}}'
```
- Send a command:
```bash
nngcat --req --dial ipc:///tmp/ubridgeReqResp --data '{"request":"sendCommand","channelID":"uThing::VOC_1234","command":{"led":true}}'
```
- Get statistics:
```bash
nngcat --req --dial ipc:///tmp/ubridgeReqResp --data '{"request":"getStatistics"}'
```

PUB/SUB stream messages are sent on topics like `/sensors/<channelID>` with a JSON body. See `src/uStreamer.cpp` and `include/ubridgeClient.h` for an end-to-end example (`consumer`).

Notes:
- `setConfig` over REQ/REP is parsed but not yet implemented for persistence.
- SUB uses prefix subscriptions; subscribing to `/sensors` receives all sensor messages.

## Platform notes
- Linux is the primary target (systemd units provided). macOS can be used for development; set `devNameBase` to `/dev/cu` to discover devices.

## Troubleshooting
- Missing NNG or LibSerial: ensure `libserial-dev` and `nng` are installed and visible to the linker (often `/usr/local/lib`). You may need `sudo ldconfig` after installing NNG.
- Permission errors on serial ports: add your user to the appropriate group (e.g. `dialout`) or run the service as root.
- No devices found: verify device nodes under `/dev/ttyACM*`/`/dev/ttyUSB*` (or supply a `devNameBase`).
- No PUB messages: confirm `streamSockUrl` matches between server and client and that your SUB topic prefix is correct.

## License
MIT. See `LICENSE`.
