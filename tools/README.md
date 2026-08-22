# Tools

Standalone utilities for working with a running shard. Nothing here is part of
the build or the docs pipeline — see `doctools/` for those.

## Memory report analyzers

`PolCore()` writes three memory reports, and these read them back. They narrow
down in this order — which column is growing, then which script, then which
variable — the same order ToGu's eScript Performance Guide describes, under
"Memory usage" (docs.polserver.com, or `docs/docs.polserver.com/pol100/include/performance.inc`
in this repo).

| Script | Reads | Answers |
|---|---|---|
| `analyze_memory_timeline.py` | `log/memoryusage.log` | which part of the server is growing, and how fast |
| `analyze_memory_usage.py` | `log/memory/scripts-*.log` | which script is holding the memory |
| `analyze_scriptmemory.py` | `log/memory/vars-*.log` | which variable inside that script |

They need nothing but Python 3.7 or newer — no third-party packages.

### Running them

Paths resolve against the working directory, so the shortest way is to `cd`
into a shard's `log/` and run them bare:

```shell
cd /path/to/shard/log

python3 ../tools/analyze_memory_timeline.py          # reads ./memoryusage.log
python3 ../tools/analyze_memory_usage.py             # reads ./memory/scripts-*.log
python3 ../tools/analyze_scriptmemory.py             # reads ./memory/vars-*.log
```

Or hand them a path. A file, a directory or a glob all work, and a directory is
tried both as the report directory itself and as the log directory holding it,
so either of these finds the same snapshots:

```shell
python3 tools/analyze_memory_usage.py /path/to/shard/log
python3 tools/analyze_memory_usage.py /path/to/shard/log/memory
python3 tools/analyze_memory_usage.py '/path/to/shard/log/memory/scripts-202608*.log'
```

Giving several snapshots at once is the normal use for the first two: one
snapshot is a reading, and the trend is what you are after.

The examples below all assume the first form — run from the shard's `log/`.

### analyze_memory_timeline.py

The wide, semicolon-separated time series that `log_memory_usage()` appends
one line to per call. Reports the latest breakdown, the biggest movers between the
first and last snapshot, whether the gap between process RSS and what the core
can account for is widening, and — for every `Count`/`Size` pair — whether you
have more objects or bigger ones, which are different bugs.

```shell
python3 ../tools/analyze_memory_timeline.py                  # ./memoryusage.log
python3 ../tools/analyze_memory_timeline.py --run all --csv  # ignore restarts, export the growth table
```

`memoryusage.log` is appended to across restarts, so growth is measured within
one server run by default; `--run` picks another (`latest`, a 1-based number,
or `all`).

### analyze_memory_usage.py

Per-script sizes from `log_script_memory()`. Streaming, so a report of any size
costs the same memory to read. Reports the largest scripts, the ones with the
most instances, the ones whose size varies most between instances, and a
per-snapshot timeline with the totals line each report ends with.

```shell
python3 ../tools/analyze_memory_usage.py                    # ./memory/scripts-*.log
python3 ../tools/analyze_memory_usage.py -n 25 --csv         # top 25, export per-script stats
```

### analyze_scriptmemory.py

Per-variable sizes from `log_script_variables( script )`, written out as a
Markdown or plain-text report rather than to the console. Groups the instances
by size, shows where each was suspended, ranks the variables, and lists the
instances that differ from the common case with a diff of what differs.

```shell
python3 ../tools/analyze_scriptmemory.py                    # ./memory/vars-*.log -> <name>_report.md
python3 ../tools/analyze_scriptmemory.py --format txt -o /tmp/report.txt
```

The script has to have been compiled with debugging information — `ecompile.cfg
GenerateDebugInfo`, or the `-x` flag — or the core has no variable names to
report and the dump says so instead.

## Reading the output

Two things the reports mean that are easy to get backwards, and that these
tools are careful about:

- **An empty column is "not measured", not zero.** `log_memory_usage( sections )`
  can measure a subset, and unmeasured columns are written empty rather than
  dropped so the header written when the file was created keeps describing it.
  Read a gap as a zero and you invent a cliff and then a recovery.
- **The allocator pools overlap `ScriptSize`, they do not add to it.** They are
  the pools eScript objects are handed out of, they only ever grow, and they are
  a high-water mark of churn rather than current usage. They are reported on
  their own and left out of the accounted total; summing every column is the
  obvious mistake.

For the same reason, the per-script lines of a `scripts-*.log` do not add up to
the totals line at its end: the script total also covers the scheduler queues
and the pid list, and the cached-program store is not itemized at all.

## A word of warning

These reports are generated on the scripts thread, which stops the server for
as long as they take. `log_memory_usage()` walks every object in the world and
every zone of every realm — a fixed six-figure count however quiet the shard
is. Generate them deliberately, from an admin command or a task on an interval
measured in minutes, never from a hot path or an event handler. Reading the
files afterwards, which is all these tools do, costs the server nothing.
