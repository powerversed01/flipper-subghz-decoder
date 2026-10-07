# Flipper Sub-GHz Listener Skeleton

This repository contains a starter app skeleton for a Flipper Zero Sub-GHz application that:

- listens on a configured frequency
- receives raw packets
- decodes a simple custom protocol
- prints decoded messages to the console/log

This is intentionally a lightweight skeleton meant to be adapted to the exact Flipper firmware branch you are building against.

## Files

- `application.fam` — metadata for the Flipper app
- `decoder.h` — decoder API
- `decoder.c` — checksum and packet-decoding logic
- `subghz_listener_app.c` — main listener loop and radio setup
- `README.md` — usage notes

## Example custom protocol

The decoder expects packets in this custom format:

- bytes[0..1] = `0xAA 0x55` start marker
- bytes[2] = payload length
- bytes[3:3+length] = payload
- bytes[3+length] = checksum = sum(payload) & 0xFF

Example:

- `AA 55 05 48 45 4C 4C 4F 74`
- decodes to `HELLO`

## Notes

- The actual Sub-GHz API names can vary between Flipper firmware versions.
- You may need to adapt includes and function names to your branch.
- This is a code skeleton for experimentation and learning, not a finished production app.

## Build notes

This project assumes you are developing inside a Flipper Zero firmware tree using the standard `fbt` build system.

Typical workflow:

1. Copy the files into your firmware app directory.
2. Ensure the app is included in your build.
3. Run the Flipper build tool.
4. Flash the app to a Flipper device.

## Next ideas

- add a proper UI screen for decoded results
- add TX support for sending custom alien-style messages
- add CRC instead of simple checksum
- add support for known protocols such as ASK/OOK packets from remotes
