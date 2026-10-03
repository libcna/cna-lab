# `consumer_canary`

Runs the template's canary and reads its verdict line back.

Every suite in this repository disposes what it creates, so none of them could
see the defect Foundation 101 found: `ContentManager` leaked its native handle
when a consumer dropped one, and `cna_game_destroy` refused the game around it.
The one thing here that behaves like a game — `cna-swift-template`'s
`HelloGame` — had been failing on **every** run for that reason, printing its
verdict line and then, on stderr:

```text
CNA Swift canary failed: CNA operation cna_game_destroy failed with result 3:
All owned C child resources must be destroyed before the game.
```

Both repositories documented the verdict and not the failure under it, because
nothing ran the canary. A canary nobody launches is a canary in a sealed jar.

```bash
python3 tools/consumer_canary/verify.py --self-test
python3 tools/consumer_canary/verify.py \
  --template ../cna-swift-template \
  --library "$CNA_NATIVE_LIBRARY" --frames 60 \
  --output docs/generated/consumer-canary-report.json
```

`--no-build` reuses the binary already in the template's `.build` instead of
building, which is what makes the check cheap enough to run every time.

## What it checks, and what it deliberately does not

The verdict line mixes three kinds of field and only two of them are
assertable.

| kind | example | treatment |
|---|---|---|
| invariant of the qualified runtime | `viewport=800x480`, every `…Refused=true` | asserted exactly |
| count tied to the request | `draws`, `updates` | `draws == requested`; `updates >= requested` |
| fact about this machine | `libraryPictures`, `connected` | read into the report, asserted nowhere |

`updates` is **not** reproducible: three runs at `--frames 600` answered 600,
601 and 602, because CNA catches a fixed time step up with extra `Update` calls
that carry no `Draw`. Short runs do not overrun and answer exactly. The
template's README used to print one of those observations as though it were the
value.

Host facts are read back rather than asserted on purpose. A gate that fails on
another machine for a reason that is not a defect is a gate people learn to
ignore.

## Falsifiability

`--self-test` covers eleven cases, and the one that matters is the shape that
hid this defect: **a clean verdict line with a failed exit status**. That must
produce one finding quoting CNA's own message, not a bare "exit 1". The others
prove a lost draw is a finding while an extra update is not, that a refusal
which stopped refusing is a finding while a differing host fact is not, and
that a missing field or a missing verdict line is caught.

The parser earns its own tests. The canary writes nested `section=field=value`
tokens (`device=isDisposed=false`), so splitting on the first `=` reads the
section name as the field — which is how the first version of this tool
reported `isDisposed` and `played` missing from a line that had both. The value
is the last `=`-separated segment and the key the one before it. `name` appears
twice in a real line, once for the adapter and once for the song; last wins.
