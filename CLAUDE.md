# CLAUDE.md

Guidance for Claude Code when working in this repository.

## What this repo is

The partner-facing distribution of `libshmmedia`, TVU's POSIX shared-memory IPC
library for moving media between processes on one machine. It is a **release and
documentation repo**, not a primary development repo — `sourceCode/` is a
snapshot, and the artifacts partners actually consume are the prebuilt packages
under `release/`.

| Path | Role |
|------|------|
| `release/` | Prebuilt SDK packages (`.tar.xz` / `.zip`). Each contains `include/`, `lib/`, `test/`, `bin/`. |
| `doc/MediaMesh_Shared_Memory_SDK.md` | The API reference. **Source of truth for the docs**; `.docx` and `.pdf` are exports of it. |
| `sampleCode/` | Mirror of the `test/` directory inside the release packages. |
| `sourceCode/libshmmedia/` | Library source snapshot. |

## Two API families

- **LibShm** (`LibShmMedia*`) — constant-sized ring items.
- **LibViShm** (`LibViShmMedia*`) — variable-sized ring items. "Vi" = variable
  item. Use this for compressed streams.

The two are parallel, not interchangeable: a segment created with
`LibViShmMediaCreate` must be read with `LibViShmMedia*` calls.

## Building anything here

Samples assume they sit inside an **extracted release package**, because the
Makefile uses `-I../include -L../lib`. Building from `sampleCode/` in the repo
root will fail — extract a release first, or pass explicit paths.

`-lz` is required and is easy to miss — the library references zlib's
`compress`/`uncompress`, so omitting it fails at link with
`undefined reference to 'compress'`. Full link line:
`-lshmmediawrap -lpthread -lrt -lz -DTVU_LINUX=1`.

At run time set `LD_LIBRARY_PATH` to the package's `lib/`, or the binaries fail
with `cannot open shared object file`.

Header availability differs between releases — `libshmmedia_variableitem_rawdata.h`
is present in 2.1.244 but **not** in 2.1.230. Samples in this repo must build
against the released packages, so do not depend on headers that only exist in
newer internal builds.

## Non-obvious behaviour worth knowing before changing code or docs

**Read/write indices wrap at `2 * item_count`.** They are not free-running
counters. Verified empirically: `item_count=300` wraps at 599, `item_count=1024`
at 2047. Any index difference must be taken modulo `2 * item_count` or it
underflows on wrap. This is not stated in the headers.

**`LibViShmMediaGetItemLength()` returns the total payload size** for
variable-item segments, not a per-item length. Use
`LibViShmMediaGetTotalPayloadSize()` when that is what you mean.

**The writer never blocks and there is no backpressure.** A slow reader silently
loses data. Critically, the read index still advances one slot per read and
*looks healthy* while the slots being read have already been overwritten —
detecting a read-index jump does not catch this. Monitor the write/read index
**lag** instead. This is documented in section 8.5.4 and implemented in
`mpegts_read_sample_code.cpp`.

**`LibShmMediaSetLogCb` is deprecated** in favour of
`LibShmMediaSetLogCallback` (which takes a `va_list`). Some older shipped
samples still use the deprecated form.

## MPEG-TS over shared memory (`tvutsshm`)

TS is **not** carried through the raw-data APIs. It goes in the user data field
of a variable-sized item, tagged with `LIBSHM_MEDIA_TYPE_MPEG_TS_DATA`
(`'TMTS'`, `0x53544D54`):

```c
ohi.i_userDataType = LIBSHM_MEDIA_TYPE_MPEG_TS_DATA;
ohi.p_userData     = ts_bytes;
ohi.i_userDataLen  = len;
LibViShmMediaSendData(h, &ohp, &ohi);      // read side: LibViShmMediaPollReadData
```

Video/audio/subtitle fields stay empty. Consumers must check `i_userDataType`
before treating the payload as TS.

**An item is always a whole number of 188-byte TS packets**, so item boundaries
fall on packet boundaries. But **the packet count per item is not fixed** — 1316
(7 × 188) is the suggested size, not a required one, and it may vary between
items. A consumer must take the length from `i_userDataLen` and never hardcode
an item size. `mpegts_write_sample_code -r` varies the packet count per item to
test exactly that.

The `tvutsshm://0?name=<name>` URL is parsed by the **application**, never by the
library. Code linking against the SDK opens a segment by name.

Reference implementation: `sampleCode/mpegts_{read,write}_sample_code.cpp`.
Contract: section 8.5 of the SDK document.

## Conventions

- Sample code is C++ but written in a C style, with the Apache 2.0 header block
  used throughout the headers. Match that when adding files.
- The Makefile in `sampleCode/` globs `*.cpp` and builds each into its own
  executable, so every sample needs its own `main()`.
- When changing the API reference, edit the `.md`. The `.docx` and `.pdf` are
  exports and will drift until regenerated — say so rather than silently leaving
  them stale.
- Release packages are binary artifacts checked into git. Do not modify them in
  place; add a new versioned file.

## Verifying a change

There is no test suite. To check that samples still work, extract a release,
build, and run a round trip:

```bash
./make_test_ts.py sample.ts
./mpegts_read_sample_code -n Verify -d 5 &
./mpegts_write_sample_code -n Verify -i sample.ts -b 10000000 -l
```

A healthy run reports the measured bitrate, zero sync errors, zero continuity
errors, and a max lag in the low single digits. `make_test_ts.py` produces
strictly correct continuity counters, so any CC error it reports is a genuine
defect in the transport path rather than in the content.
