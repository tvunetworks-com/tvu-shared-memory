# tvu-shared-memory

A sub-project of MediaMesh demonstrating how to interact with TVU's shared
memory system using the provided SDK.

## Layout

| Path | Contents |
|------|----------|
| `release/` | Prebuilt SDK packages. Extract one to get `include/`, `lib/` and `test/`. |
| `doc/` | `MediaMesh_Shared_Memory_SDK.md` — full API reference (also as `.docx` / `.pdf`). |
| `sampleCode/` | Reference programs, mirrored from the `test/` directory of the release packages. |
| `sourceCode/` | `libshmmedia` source tree. |

## Getting started

1. Extract a package from `release/` — the shared library is under `lib/` and
   the headers under `include/`.
2. Build the samples from inside the extracted package, so that `../include` and
   `../lib` resolve:

   ```bash
   cd test && make
   ```

3. Point the loader at the library before running:

   ```bash
   export LD_LIBRARY_PATH=$PWD/../lib
   ```

Linking requires `-lshmmediawrap -lpthread -lrt -lz` and `-DTVU_LINUX=1` on
Linux. `-lz` is not optional — the library references zlib's `compress` /
`uncompress`. See section 2 of the SDK document for details.

## Sample programs

**Media** — video, audio, subtitle and extension data:

- `write_sample_code.cpp` — writer example
- `read_sample_code.cpp` — reader example

**MPEG-TS** — a muxed transport stream carried over shared memory, tagged with
`LIBSHM_MEDIA_TYPE_MPEG_TS_DATA` (`'TMTS'`). See section 8.5 of the SDK document
for the transport contract, including the flow-control rules a consumer must
observe:

- `mpegts_write_sample_code.cpp` — feed a TS file or stdin into a segment
- `mpegts_read_sample_code.cpp` — attach, validate TS structure, detect
  reader-side data loss, optionally write the recovered stream out
- `make_test_ts.py` — generate a synthetic TS with strictly correct continuity
  counters, for verifying an integration before using real content

```bash
./make_test_ts.py sample.ts
./mpegts_read_sample_code  -n LiveTransmit -v                          # terminal 1
./mpegts_write_sample_code -n LiveTransmit -i sample.ts -b 8000000 -l  # terminal 2
```

## Releases

Packages live in [`release/`](release/). The most recent are
`ShmmediaReleaseForTvuPartner-win64-2.1.231.zip` and
`ShmmediaReleaseForTVUPartner-x86_64-2.1.230.tar.xz`.
