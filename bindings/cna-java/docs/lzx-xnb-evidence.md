# Managed XNA LZX XNB evidence

## Scope and authority

This implementation handles the compression framing used by XNA 4.0 Windows XNB v5. It is not a
generic promise that every container carrying LZX-like bytes is accepted. The XNB header remains
authoritative for platform, version, flags, file size, and decompressed size; only the payload is
replaced by the exact decompressed byte stream before normal ContentReader parsing.

The uncompressed flag path was not refactored. The compressed flag selects
`XnbLzxDecompression`, which owns one stateful `LzxDecoder` for the whole asset. LZ4 and unknown flag
combinations continue to fail explicitly.

## Framing implemented

After the 14-byte compressed XNB header, each ordinary frame begins with a 2-byte big-endian
compressed block length. An `0xFF` first byte selects the extended header containing a 2-byte
decompressed frame length followed by the 2-byte compressed block length. Ordinary frames target
32 KiB. The decoder retains a 64-KiB LZX window and repeated-offset state across frames.

The parser enforces:

- complete short or extended frame headers;
- non-negative, available compressed block lengths;
- frame lengths no larger than 32 KiB and no larger than the remaining declared output;
- exact per-frame decoder output;
- exact equality with the XNB decompressed-size field;
- only the canonical zero end marker/padding after output completion;
- no read beyond the compressed XNB file size.

The LZX core handles verbatim, aligned, and uncompressed blocks, canonical Huffman pretree/main/
length/aligned tables, repeated match offsets, position slots, Intel E8 preprocessing state, and
window wrap. Invalid trees, distances, matches, block sizes, bitstream exhaustion, and output
overrun fail with `ContentLoadException` rather than yielding partial content.

## Deterministic fixtures and failure matrix

Tests generate legal LZX uncompressed blocks inside XNA short/extended frames. These fixtures are
small, deterministic, redistributable, and enter the real reader table. Coverage includes:

| Case | Expected result |
|---|---|
| single compressed frame | normal managed object load |
| multiple frames with retained decoder state | normal managed object load |
| ordinary 32-KiB frame | normal managed object load |
| Texture2D compressed vs uncompressed | same Texture2D reader/native upload path |
| declared output exact | accepted |
| output shorter/longer than declaration | rejected |
| truncated short/extended header | rejected |
| truncated compressed block | rejected |
| zero/oversized/malformed frame length | rejected |
| malformed block length | rejected |
| invalid non-zero trailing framing | rejected |
| invalid LZX data | rejected |
| reader throws after decompression | reader failure propagated; constructed resources rolled back |
| repeated compressed Load | same ContentManager cache identity |
| compressed Unload | resource closed through ordinary reverse ownership graph |

As an independent check beyond generated uncompressed LZX blocks, two existing read-only legal
compressed XNB fixtures produced byte-exact known outputs of 16,561 and 44,032 bytes. They were used
only at verification time and were not copied into this repository.

## Deliberate non-claims

This does not add public XNA reader types, dispatch on asset names, call a loose-file loader, decode
or relabel unsupported texture formats, or claim arbitrary Model dependency-reader coverage.
Successful decompression proves framing; the selected ContentTypeReader must still support the
asset's actual reader graph and native resource formats.
