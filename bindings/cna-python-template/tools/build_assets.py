#!/usr/bin/env python3
"""Build the tiny legal synthetic XNB canary committed by this template."""

from __future__ import annotations

from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parents[1]
READER = "Microsoft.Xna.Framework.Content.Texture2DReader, Microsoft.Xna.Framework"


def seven(value: int) -> bytes:
    result = bytearray()
    while value >= 0x80:
        result.append((value & 0x7F) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)


def text(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return seven(len(encoded)) + encoded


def texture_xnb() -> bytes:
    pixels = bytes((
        255, 255, 255, 255, 255, 128, 0, 255,
        255, 128, 0, 255, 255, 255, 255, 255,
    ))
    payload = bytearray(seven(1))
    payload.extend(text(READER))
    payload.extend(struct.pack("<i", 0))       # reader version
    payload.extend(seven(0))                    # shared-resource count
    payload.extend(seven(1))                    # root reader index
    payload.extend(struct.pack("<iIII", 0, 2, 2, 1))  # Color, width, height, levels
    payload.extend(struct.pack("<I", len(pixels)))
    payload.extend(pixels)
    return b"XNBw\x05\x00" + struct.pack("<I", len(payload) + 10) + payload


def main() -> None:
    destination = ROOT / "Content" / "logo.xnb"
    destination.write_bytes(texture_xnb())
    print(f"wrote {destination.relative_to(ROOT)} ({destination.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
