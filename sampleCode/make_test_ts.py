#!/usr/bin/env python3
"""
Generate a synthetic MPEG-TS file with strictly correct continuity counters.

Useful for verifying an integration end to end without needing real content:
any continuity error the reader reports against this file is a genuine defect
in the transport path, not in the source.

    ./make_test_ts.py sample.ts
"""
import sys

PIDS = [(0x0000, 1),        # PAT
        (0x0100, 20),       # video
        (0x0101, 4),        # audio
        (0x1FFF, 2)]        # null / stuffing
CYCLES = 1400               # ~7 MB

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "sample.ts"
    cc = {pid: 0 for pid, _ in PIDS}
    out = bytearray()

    for _ in range(CYCLES):
        for pid, count in PIDS:
            for _ in range(count):
                pkt = bytearray(b"\xff" * 188)
                pkt[0] = 0x47
                pkt[1] = (pid >> 8) & 0x1F
                pkt[2] = pid & 0xFF
                pkt[3] = 0x10 | cc[pid]         # afc=1 (payload only) + CC
                out += pkt
                if pid != 0x1FFF:               # null packets do not advance CC
                    cc[pid] = (cc[pid] + 1) & 0x0F

    with open(path, "wb") as f:
        f.write(out)

    print(f"wrote {path}: {len(out)} bytes, {len(out) // 188} TS packets")

if __name__ == "__main__":
    main()
