#!/usr/bin/env python3
"""
analyze_memory_usage.py
Analyzes POL's per-script memory dumps for script memory usage statistics.
Streaming parser — memory usage stays constant regardless of file size.

Input: what PolCore().log_script_memory() writes — one timestamped file per
snapshot under log/memory/ (scripts-YYYYMMDD-HHMMSS.log). Older cores, where this
was PolCore().internal(5), appended every snapshot to a single
log/memoryusagescripts.log instead; that is accepted too, and so is a directory or
a glob. The cross-snapshot trend and growth analysis needs several snapshots to say
anything, so pointing this at the whole memory/ directory is the normal use.
"""

import os
import re
import sys
import csv
import glob as globmod
import argparse
from pathlib import Path
from datetime import datetime

# The report draws box characters, which a Windows console defaulting to cp1252
# cannot encode - it raises rather than degrading, killing the run.
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, ValueError):  # not a reconfigurable stream
        pass

# Where log_script_memory() puts its snapshots, and the pre-POL100.3.0 file.
SNAPSHOT_DIR  = "memory"
SNAPSHOT_GLOB = "scripts-*.log"
LEGACY_LOG    = "memoryusagescripts.log"


def resolve_inputs(spec: str) -> list:
    """Turn a file, directory or glob into a sorted list of snapshot files.

    Names carry a YYYYMMDD-HHMMSS stamp, so sorting by name is chronological.
    """
    if spec is None:
        base, candidates = Path("."), (Path(SNAPSHOT_DIR), Path("."))
    else:
        path = Path(spec)
        if path.is_dir():
            # Either the snapshot directory itself or the log directory holding
            # it, so pointing at log/ works as well as log/memory/.
            base, candidates = path, (path / SNAPSHOT_DIR, path)
        elif path.exists():
            return [path]
        else:
            return sorted(Path(q) for q in globmod.glob(spec))

    for candidate in candidates:
        found = sorted(candidate.glob(SNAPSHOT_GLOB))
        if found:
            return found
    legacy = base / LEGACY_LOG
    return [legacy] if legacy.exists() else []

# ── ANSI colors ───────────────────────────────────────────────────────────────
class C:
    CYAN   = "\033[96m"
    YELLOW = "\033[93m"
    GREEN  = "\033[92m"
    GRAY   = "\033[90m"
    RESET  = "\033[0m"

# Escape codes in a redirected report are noise. NO_COLOR is the cross-tool
# convention for asking for none.
USE_COLOR = sys.stdout.isatty() and "NO_COLOR" not in os.environ


def c(color: str, text: str) -> str:
    return f"{color}{text}{C.RESET}" if USE_COLOR else text

# ── Patterns ──────────────────────────────────────────────────────────────────
RE_TIMESTAMP = re.compile(r"^\[(\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2})\]")
RE_SECTION   = re.compile(r"^\s*(\w+):\s*$")
RE_ENTRY     = re.compile(r"^\s*(\S+\.ecl)\s+(\d+)\s*$")
# Trailing summary line, added in POL100.3.0. Worth capturing rather than
# skipping: the cached-program total is not itemized anywhere above it, so the
# sum of the per-script entries never accounts for it.
RE_SUMMARY   = re.compile(
    r"^\s*(\d+) scripts?, (\d+) bytes; (\d+) cached programs?, (\d+) bytes\s*$"
)

# ── Streaming aggregator ──────────────────────────────────────────────────────
class Aggregator:
    """
    Accumulates per-script stats and per-snapshot totals without storing
    individual entries. Memory grows only with the number of unique script
    paths and snapshot count — not with file size.
    """

    def __init__(self):
        self._scripts: dict  = {}
        self._sections: dict = {}
        self.snapshots: list = []
        self.total_memory    = 0
        self.total_instances = 0
        self._snap_count     = 0
        self._snap_total     = 0
        self._snap_store     = None
        self._snap_reported  = None
        self._current_ts     = None

    def feed(self, script: str, memory: int, section):
        # Script stats
        if script not in self._scripts:
            self._scripts[script] = {"instances": 0, "total": 0, "min": memory, "max": memory}
        s = self._scripts[script]
        s["instances"] += 1
        s["total"]     += memory
        if memory < s["min"]: s["min"] = memory
        if memory > s["max"]: s["max"] = memory

        # Section stats
        if section:
            if section not in self._sections:
                self._sections[section] = {"instances": 0, "total": 0}
            sec = self._sections[section]
            sec["instances"] += 1
            sec["total"]     += memory

        self._snap_count     += 1
        self._snap_total     += memory
        self.total_memory    += memory
        self.total_instances += 1

    def _close_snapshot(self):
        self.snapshots.append({
            "timestamp": self._current_ts,
            "count":     self._snap_count,
            "total":     self._snap_total,
            "store":     self._snap_store,
            "reported":  self._snap_reported,
        })

    def new_snapshot(self, timestamp: str):
        if self._current_ts is not None:
            self._close_snapshot()
        self._current_ts    = timestamp
        self._snap_count    = 0
        self._snap_total    = 0
        self._snap_store    = None
        self._snap_reported = None

    def report_totals(self, script_bytes: int, store_bytes: int):
        """Authoritative totals from the snapshot's summary line, if present.

        The scheduler's own total also covers its queue and pid-list overhead,
        which no per-script line accounts for, so it sits above the itemized
        sum. The cached-program total is not itemized at all.
        """
        self._snap_reported = script_bytes
        self._snap_store    = store_bytes

    def flush(self):
        # No count test: a snapshot taken while every queue was empty says
        # "nothing was running", and mid-file ones are already recorded.
        if self._current_ts is not None:
            self._close_snapshot()
            self._current_ts = None

    def script_stats(self) -> list:
        result = []
        for name, s in self._scripts.items():
            avg = s["total"] / s["instances"]
            result.append({
                "script":    name,
                "instances": s["instances"],
                "total":     s["total"],
                "avg":       round(avg),
                "min":       s["min"],
                "max":       s["max"],
                "growth":    s["max"] - s["min"],
            })
        return result

    def section_stats(self) -> list:
        return [{"section": k, **v} for k, v in self._sections.items()]

    @property
    def unique_scripts(self) -> int:
        return len(self._scripts)


# ── Streaming parser ──────────────────────────────────────────────────────────
def parse_logs(paths: list, agg: Aggregator, progress: bool):
    """Feed every snapshot file into the aggregator, oldest first.

    A per-snapshot file opens with its own timestamp header, so snapshots keep
    being delimited the same way whether they arrive as one file each or all
    together in a legacy log.
    """
    total_bytes = sum(p.stat().st_size for p in paths) or 1
    bytes_read  = 0
    # Bucketed rather than tested for divisibility: one line can carry the count
    # past a multiple of five, and "pct % 5" would then print nothing at all.
    # st_size counts the separators text mode collapses, so clamp the estimate.
    last_bucket = -1

    for path in paths:
        current_section = None
        # A legacy file opens its snapshots itself; a per-snapshot file may in
        # principle be headerless, so make sure entries always land somewhere.
        seen_header = False

        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            for line in fh:
                bytes_read += len(line.encode("utf-8", errors="replace"))
                line = line.rstrip()

                if progress:
                    bucket = min(100, int(bytes_read / total_bytes * 100)) // 5
                    if bucket != last_bucket:
                        last_bucket = bucket
                        print(f"\r  Reading... {bucket * 5}%", end="", flush=True)

                m = RE_TIMESTAMP.match(line)
                if m:
                    agg.new_snapshot(m.group(1))
                    current_section = None
                    seen_header = True
                    continue

                m = RE_SUMMARY.match(line)
                if m:
                    agg.report_totals(int(m.group(2)), int(m.group(4)))
                    continue

                m = RE_SECTION.match(line)
                if m:
                    current_section = m.group(1)
                    continue

                m = RE_ENTRY.match(line)
                if m:
                    if not seen_header:
                        agg.new_snapshot(path.stem)
                        seen_header = True
                    agg.feed(m.group(1), int(m.group(2)), current_section)

    agg.flush()
    if progress:
        print("\r  Reading... 100%", flush=True)


# ── Display helpers ───────────────────────────────────────────────────────────
def rule(label: str):
    print(c(C.YELLOW, f"── {label} {'─' * max(0, 57 - len(label))}"))

def truncate(s: str, width: int) -> str:
    if len(s) <= width:
        return s
    if width <= 3:      # no room for the ellipsis, let alone anything after it
        return s[-width:] if width > 0 else ""
    return "..." + s[-(width - 3):]

def print_table(rows, columns):
    headers = [h.ljust(w) if not r else h.rjust(w) for h, _, w, r in columns]
    sep     = "  ".join("-" * w for _, _, w, _ in columns)
    print("  " + "  ".join(headers))
    print("  " + sep)
    for row in rows:
        cells = []
        for _, key, w, right in columns:
            val = str(row[key])
            cells.append(val.rjust(w) if right else val.ljust(w))
        print("  " + "  ".join(cells))
    print()


# ── Snapshot timeline helpers ─────────────────────────────────────────────────
def print_snapshot_summary(snapshots: list, max_rows: int = 20):
    n = len(snapshots)
    if n == 0:
        return

    rule("SNAPSHOT TIMELINE")

    cols = [
        ("Timestamp",     "timestamp", 18, False),
        ("Instances",     "count",      9, True),
        ("Itemized (MB)", "mb_f",      13, True),
        ("Reported (MB)", "rep_f",     13, True),
        ("Store (MB)",    "store_f",   10, True),
    ]

    def mb(v):
        return f"{v/1024/1024:.2f}" if v is not None else "-"

    def enrich(rows):
        return [{**r,
                 "mb_f":    mb(r["total"]),
                 "rep_f":   mb(r.get("reported")),
                 "store_f": mb(r.get("store"))}
                for r in rows]

    half = max_rows // 2

    if n <= max_rows:
        print_table(enrich(snapshots), cols)
    else:
        print(c(C.GRAY, f"  ({n:,} snapshots total — showing first {half} and last {half})\n"))
        print_table(enrich(snapshots[:half]), cols)
        print(c(C.GRAY, f"  ... {n - max_rows:,} snapshots omitted ...\n"))
        print_table(enrich(snapshots[-half:]), cols)

    # "Reported" is what the scheduler totalled for itself; it also covers the
    # queue and pid-list overhead that no per-script line accounts for, so a
    # small gap above the itemized sum is expected rather than a parse error.
    gaps = [s["reported"] - s["total"] for s in snapshots
            if s.get("reported") is not None]
    if gaps:
        print(c(C.GRAY,
            f"  Unattributed (reported - itemized): "
            f"{min(gaps)/1024:,.0f} - {max(gaps)/1024:,.0f} KB per snapshot; "
            f"the cached-program store is counted separately again.\n"))

    # Trend: compare first quarter vs last quarter
    q         = max(1, n // 4)
    first_avg = sum(s["total"] for s in snapshots[:q]) / q
    last_avg  = sum(s["total"] for s in snapshots[-q:]) / q
    delta     = last_avg - first_avg
    pct       = (delta / first_avg * 100) if first_avg else 0
    arrow     = "^" if delta > 0 else ("v" if delta < 0 else "-")
    trend_col = C.YELLOW if delta > 0 else (C.GREEN if delta < 0 else C.GRAY)
    sign      = "+" if delta >= 0 else ""
    print(c(trend_col,
        f"  Trend (first {q} vs last {q} snapshots): "
        f"{arrow} {abs(pct):.1f}%  "
        f"({sign}{delta:,.0f} B avg per snapshot)\n"
    ))


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(
        description="Analyze POL per-script memory snapshots (streaming, large-file safe)."
    )
    parser.add_argument(
        "logfile",
        nargs="?",
        default=None,
        metavar="PATH",
        help=f"Snapshot file, directory or glob. Default: {SNAPSHOT_DIR}/{SNAPSHOT_GLOB}, "
             f"falling back to ./{SNAPSHOT_GLOB} then the legacy {LEGACY_LOG}. "
             f"Give a directory to analyze every snapshot in it.",
    )
    parser.add_argument(
        "-n", "--top",
        type=int,
        default=10,
        metavar="N",
        help="How many top entries to show (default: 10)",
    )
    parser.add_argument(
        "--csv",
        action="store_true",
        help="Export per-script stats to a CSV file next to the log",
    )
    parser.add_argument(
        "--no-progress",
        action="store_true",
        help="Suppress the reading progress indicator",
    )
    parser.add_argument(
        "--no-color",
        action="store_true",
        help="Never emit ANSI colors (already off when stdout is not a terminal)",
    )
    args = parser.parse_args()

    if args.no_color:
        globals()["USE_COLOR"] = False

    log_paths = resolve_inputs(args.logfile)
    if not log_paths:
        where = args.logfile or f"{SNAPSHOT_DIR}/{SNAPSHOT_GLOB} (or ./{LEGACY_LOG})"
        print(c(C.YELLOW, f"Error: no snapshot files found: {where}"))
        print(c(C.GRAY, "  PolCore().log_script_memory() writes them under log/memory/."))
        raise SystemExit(1)

    total_mb = sum(p.stat().st_size for p in log_paths) / 1024 / 1024

    print()
    print(c(C.CYAN, "=== Memory Usage Log Analyzer ==="))
    if len(log_paths) == 1:
        print(c(C.GRAY, f"File   : {log_paths[0].resolve()}"))
    else:
        print(c(C.GRAY, f"Files  : {len(log_paths)} snapshots in {log_paths[0].parent.resolve()}"))
        print(c(C.GRAY, f"         {log_paths[0].name} .. {log_paths[-1].name}"))
    print(c(C.GRAY, f"Size   : {total_mb:.1f} MB"))
    print(c(C.GRAY, f"Parsed : {datetime.now():%Y-%m-%d %H:%M:%S}"))
    print()

    agg = Aggregator()
    parse_logs(log_paths, agg, progress=not args.no_progress)
    print()

    if agg.total_instances == 0:
        print(c(C.YELLOW, "No script entries found in the log file."))
        raise SystemExit(0)

    script_stats  = agg.script_stats()
    section_stats = sorted(agg.section_stats(), key=lambda x: x["total"], reverse=True)
    N             = args.top

    # ── Overall summary ───────────────────────────────────────────────────────
    rule("OVERALL SUMMARY")
    print(f"  Snapshots (timestamps) : {len(agg.snapshots):,}")
    print(f"  Total script instances : {agg.total_instances:,}")
    print(f"  Unique scripts         : {agg.unique_scripts:,}")
    print(f"  Total memory (all)     : {agg.total_memory:,} bytes  ({agg.total_memory/1024:.2f} KB)")
    print()

    # ── Top N by total memory ─────────────────────────────────────────────────
    rule(f"TOP {N} SCRIPTS BY TOTAL MEMORY")
    top_mem = sorted(script_stats, key=lambda x: x["total"], reverse=True)[:N]
    rows = [{**r, "script": truncate(r["script"], 58),
             "total_f": f"{r['total']:,}", "avg_f": f"{r['avg']:,}",
             "min_f": f"{r['min']:,}", "max_f": f"{r['max']:,}"} for r in top_mem]
    print_table(rows, [
        ("Script",    "script",   58, False),
        ("Instances", "instances", 9, True),
        ("Total (B)", "total_f",  12, True),
        ("Avg (B)",   "avg_f",    9, True),
        ("Min (B)",   "min_f",    9, True),
        ("Max (B)",   "max_f",    9, True),
    ])

    # ── Top N by instance count ───────────────────────────────────────────────
    rule(f"TOP {N} SCRIPTS BY INSTANCE COUNT")
    top_inst = sorted(script_stats, key=lambda x: x["instances"], reverse=True)[:N]
    rows = [{**r, "script": truncate(r["script"], 58),
             "total_f": f"{r['total']:,}", "avg_f": f"{r['avg']:,}"} for r in top_inst]
    print_table(rows, [
        ("Script",    "script",   58, False),
        ("Instances", "instances", 9, True),
        ("Total (B)", "total_f",  12, True),
        ("Avg (B)",   "avg_f",    9, True),
    ])

    # ── Memory growth suspects ────────────────────────────────────────────────
    rule("MEMORY GROWTH SUSPECTS (Max - Min > 0)")
    suspects = sorted(
        [s for s in script_stats if s["growth"] > 0],
        key=lambda x: x["growth"], reverse=True
    )[:N]
    if suspects:
        rows = [{**r, "script": truncate(r["script"], 58),
                 "min_f": f"{r['min']:,}", "max_f": f"{r['max']:,}",
                 "growth_f": f"{r['growth']:,}"} for r in suspects]
        print_table(rows, [
            ("Script",     "script",   58, False),
            ("Instances",  "instances", 9, True),
            ("Min (B)",    "min_f",     9, True),
            ("Max (B)",    "max_f",     9, True),
            ("Growth (B)", "growth_f", 11, True),
        ])
    else:
        print(c(C.GREEN, "  No memory growth detected across instances.\n"))

    # ── Snapshot timeline ─────────────────────────────────────────────────────
    if len(agg.snapshots) > 1:
        print_snapshot_summary(agg.snapshots)

    # ── Section breakdown ─────────────────────────────────────────────────────
    if section_stats:
        rule("MEMORY BY SECTION")
        rows = [{**s, "total_f": f"{s['total']:,}",
                 "kb_f": f"{s['total']/1024:.2f}"} for s in section_stats]
        print_table(rows, [
            ("Section",    "section",   24, False),
            ("Instances",  "instances",  9, True),
            ("Total (B)",  "total_f",   12, True),
            ("Total (KB)", "kb_f",      10, True),
        ])

    # ── CSV export ────────────────────────────────────────────────────────────
    if args.csv:
        # Several snapshots collapse into one set of stats, so the export is
        # named for the run rather than for any single input file.
        csv_path = (log_paths[0].with_suffix(".csv") if len(log_paths) == 1
                    else log_paths[0].parent / "scripts-combined.csv")
        with open(csv_path, "w", newline="", encoding="utf-8") as fh:
            writer = csv.DictWriter(fh, fieldnames=[
                "script", "instances", "total", "avg", "min", "max", "growth"
            ])
            writer.writeheader()
            for row in sorted(script_stats, key=lambda x: x["total"], reverse=True):
                writer.writerow(row)
        print(c(C.GREEN, f"  CSV exported to: {csv_path}\n"))

    print(c(C.CYAN, "=== Analysis complete ===\n"))


if __name__ == "__main__":
    main()
