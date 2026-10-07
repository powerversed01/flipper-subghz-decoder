# Flipper Sub-GHz TX/RX Full App

This is a complete Flipper Zero Sub-GHz application with:

- **RX mode**: listen for and decode incoming Sub-GHz packets
- **TX mode**: compose and transmit custom messages
- **Live display**: shows received and sent messages on screen
- **Custom protocol**: simple packet format with checksum validation

## Features

### Display
- Shows current mode (TX or RX)
- Displays last 6 messages received/sent
- Real-time updates

### RX Mode
- Continuously listens on 433.92 MHz
- ASK/OOK modulation
- Decodes custom protocol packets
- Validates checksums
- Displays decoded messages on screen

### TX Mode
- Press OK to enter text input
- Type a message (up to 63 characters)
- Message is encoded with custom protocol
- Sent over Sub-GHz radio
- Appears in message log

### Controls
- **UP**: Toggle between RX and TX modes
- **OK**: In TX mode, enter message input
- **BACK**: Exit app

## Custom Protocol Format

```
Byte 0-1:    0xAA 0x55          (start marker)
Byte 2:      payload_length     (1-63)
Byte 3+:     payload             (message text)
Byte 3+L:    checksum            (sum of payload & 0xFF)
```

### Example Packet

```
AA 55 05 48 45 4C 4C 4F 74
  |  |  |  |              |
  |  |  |  +-- "HELLO"    +-- checksum (0x74)
  |  |  +---- length: 5
  |  +------- magic: 0x55
  +---------- magic: 0xAA
```

## Files

- `application.fam` — app metadata
- `subghz_app.h` — app state and structs
- `subghz_app.c` — app helper functions
- `decoder.h` — decoder API
- `decoder.c` — packet encoding/decoding logic
- `subghz_app_main.c` — main app loop, UI, and radio worker
- `README.md` — this file

## Build & Run

Assuming you have the Flipper Zero firmware and `fbt` build tool set up:

```bash
# Copy app files into your firmware app directory
cp -r flipper-subghz-decoder /path/to/firmware/applications/external/subghz_txrx

# Build
cd /path/to/firmware
./fbt fap_subghz_txrx

# Flash to device
./fbt flash_usb
```

Then open the app from your Flipper Zero menu: **Applications → Sub-GHz → SubGHz TX/RX**

## Testing Two Flippers

1. On Flipper A: set to **RX mode** → app listens
2. On Flipper B: set to **TX mode** → press OK → type message → send
3. Message appears on Flipper A's display

## Notes

- Frequency is hardcoded to 433.92 MHz (modify `APP_FREQ` in `subghz_app_main.c`)
- Modulation is ASK/OOK (configure in `subghz_app_main.c`)
- This is a skeleton/demo app — real RF transmission is simplified
- Adjust API calls to match your Flipper firmware version
- Always check local regulations before transmitting on any frequency

## Future Enhancements

- [ ] Configurable frequency selection
- [ ] Modulation selection (FSK, OOK, etc.)
- [ ] Message history save/load
- [ ] CRC-16 instead of simple checksum
- [ ] Signal strength indicator
- [ ] Frequency scanner mode
- [ ] Known protocol decoder integration (remotes, sensors, etc.)

## License

MIT or similar — modify as needed for your project.
