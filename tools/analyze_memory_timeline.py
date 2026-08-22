#!/usr/bin/env python3
"""
analyze_memory_timeline.py
Analyzes POL's memoryusage.log — the wide, semicolon-separated time series that
PolCore().log_memory_usage() appends one line to per snapshot.

This is the "where is the memory going, and is it still going there" view.
The per-script drill-downs are separate tools:
  analyze_memory_usage.py   - per-script sizes  (memory/scripts-*.log)
  analyze_scriptmemory.py   - per-variable sizes (memory/vars-*.log)

Two things this file needs care with:

* Empty fields. A column can be written empty rather than dropped, so that the
  header written when the file was created keeps describing it. An empty field
  means "not measured", which is not zero — reading it as zero invents a cliff
  and then a recovery.

* The allocator columns are pools that eScript objects are handed out of, so
  they overlap ScriptSize rather than adding to it. They are reported on their
  own and left out of the accounted total.
"""

import os
import re
import sys
import csv
import argparse
from pathlib import Path
from datetime import datetime

# The report draws box characters, which a Windows console defaulting to cp1252
# cannot encode - it raises rather than degrading, killing the run.
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, ValueError):
        pass

DEFAULT_LOG = "memoryusage.log"
RE_TIME = re.compile(r"^\[(\d{2})/(\d{2})\s+(\d{2}):(\d{2}):(\d{2})\]")


class C:
    CYAN   = "\033[96m"
    YELLOW = "\033[93m"
    GREEN  = "\033[92m"
    RED    = "\033[91m"
    GRAY   = "\033[90m"
    RESET  = "\033[0m"


# Escape codes in a redirected report are noise. NO_COLOR is the cross-tool
# convention for asking for none.
USE_COLOR = sys.stdout.isatty() and "NO_COLOR" not in os.environ


def c(color, text):
    return f"{color}{text}{C.RESET}" if USE_COLOR else text


# ── Parsing ───────────────────────────────────────────────────────────────────
def parse_log(path):
    """Return (columns, rows). Each row is {"time": dt|None, col: int|None}."""
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = [ln.rstrip("\n\r") for ln in fh if ln.strip()]

    if not lines:
        return [], []

    header = [h.strip() for h in lines[0].split(";")]
    if not header or header[0].lower() != "time":
        raise ValueError(
            "first line is not a memoryusage.log header (expected it to start "
            f"with 'Time'), got: {lines[0][:60]!r}"
        )
    columns = header[1:]

    rows = []
    # The stamp carries no year. Assume the log does not span more than a year
    # and roll forward whenever the clock goes backwards.
    year = datetime.now().year
    prev_dt = None

    for line in lines[1:]:
        parts = [p.strip() for p in line.split(";")]
        m = RE_TIME.match(parts[0])
        dt = None
        if m:
            mo, day, hh, mm, ss = (int(g) for g in m.groups())
            try:
                dt = datetime(year, mo, day, hh, mm, ss)
            except ValueError:      # 02/29 in a non-leap year
                dt = None
            if dt and prev_dt and dt < prev_dt:
                year += 1
                dt = dt.replace(year=year)
            if dt:
                prev_dt = dt

        row = {"time": dt, "raw_time": parts[0]}
        for idx, col in enumerate(columns, start=1):
            cell = parts[idx] if idx < len(parts) else ""
            row[col] = int(cell) if cell.lstrip("-").isdigit() else None
        rows.append(row)

    return columns, rows


# ── Column classification ─────────────────────────────────────────────────────
def is_count(col):
    return col.endswith("Count")


def is_allocator(col):
    return col.endswith("AllocatorSize")


def accounted_columns(columns):
    """Size columns that sum to a meaningful total.

    ProcessSize is the thing being explained, not part of the explanation, and
    the allocator pools overlap ScriptSize.
    """
    return [col for col in columns
            if not is_count(col)
            and col != "ProcessSize"
            and not is_allocator(col)]


def split_runs(columns, rows):
    """Split the log at server restarts.

    memoryusage.log is appended to across restarts, so a naive first-vs-last
    comparison can span several lifetimes of the process and report a fall in
    memory that is really just a fresh start. The allocator pools never shrink
    within one process — they are reserve counters, not live usage — so any
    drop in one marks a restart.
    """
    alloc = [col for col in columns if is_allocator(col)]
    if not rows:
        return []
    runs = [[rows[0]]]
    for prev, cur in zip(rows, rows[1:]):
        restarted = any(
            prev.get(a) is not None and cur.get(a) is not None and cur[a] < prev[a]
            for a in alloc
        )
        runs.append([cur]) if restarted else runs[-1].append(cur)
    return runs


def pairs(columns):
    """[(base, countcol, sizecol)] for every Count column with a Size twin."""
    out = []
    for col in columns:
        if is_count(col):
            base = col[:-len("Count")]
            twin = base + "Size"
            if twin in columns:
                out.append((base, col, twin))
    return out


# ── Formatting ────────────────────────────────────────────────────────────────
def human(n):
    """Bytes as a human-readable size; None as a gap."""
    if n is None:
        return "-"
    neg = n < 0
    n = abs(n)
    for unit, div in (("TB", 1 << 40), ("GB", 1 << 30), ("MB", 1 << 20), ("KB", 1 << 10)):
        if n >= div:
            return f"{'-' if neg else ''}{n/div:.2f} {unit}"
    return f"{'-' if neg else ''}{n} B"


def num(n):
    return "-" if n is None else f"{n:,}"


def signed(n, suffix=""):
    """Signed integer, but a plain 0 for exactly zero rather than '+0'."""
    if n is None:
        return "-"
    n = int(n)
    return f"{n:,}{suffix}" if n == 0 else f"{n:+,}{suffix}"


def signed_human(n, suffix=""):
    """As signed(), for byte counts."""
    if n is None:
        return "-"
    if n == 0:
        return "0 B" + suffix
    return ("+" if n > 0 else "") + human(int(n)) + suffix


def pct(delta, base):
    if base in (None, 0) or delta is None:
        return "-"
    v = delta / base * 100
    return "0.0%" if delta == 0 else f"{v:+.1f}%"


def rule(label):
    print(c(C.YELLOW, f"── {label} {'─' * max(0, 60 - len(label))}"))


def print_table(rows, columns):
    if not rows:
        return
    widths = []
    for head, key, right in columns:
        widest = max([len(str(r.get(key, ""))) for r in rows] + [len(head)])
        widths.append(widest)
    head_cells = [h.rjust(w) if r else h.ljust(w)
                  for (h, _k, r), w in zip(columns, widths)]
    print("  " + "  ".join(head_cells))
    print("  " + "  ".join("-" * w for w in widths))
    for row in rows:
        cells = []
        for (_h, key, right), w in zip(columns, widths):
            val = str(row.get(key, ""))
            cells.append(val.rjust(w) if right else val.ljust(w))
        print("  " + "  ".join(cells))
    print()


# ── Series helpers ────────────────────────────────────────────────────────────
def series(rows, col):
    """Measured (row, value) points for a column, gaps dropped."""
    return [(r, r[col]) for r in rows if r.get(col) is not None]


def hours_between(a, b):
    if not a or not b:
        return None
    delta = (b - a).total_seconds() / 3600.0
    return delta if delta > 0 else None


def growth_row(rows, col):
    pts = series(rows, col)
    if len(pts) < 2:
        if len(pts) == 1:
            return {"column": col, "first": pts[0][1], "last": pts[0][1],
                    "delta": 0, "samples": 1, "per_hour": None}
        return None
    (first_row, first), (last_row, last) = pts[0], pts[-1]
    span = hours_between(first_row["time"], last_row["time"])
    delta = last - first
    return {
        "column":   col,
        "first":    first,
        "last":     last,
        "delta":    delta,
        "samples":  len(pts),
        "per_hour": (delta / span) if span else None,
    }


# ── Report sections ───────────────────────────────────────────────────────────
def report_overview(path, columns, rows):
    stamped = [r for r in rows if r["time"]]
    print()
    print(c(C.CYAN, "=== POL Memory Timeline ==="))
    print(c(C.GRAY, f"File      : {path.resolve()}"))
    print(c(C.GRAY, f"Snapshots : {len(rows):,}  ({len(columns)} columns)"))
    if len(stamped) >= 2:
        span = stamped[-1]["time"] - stamped[0]["time"]
        print(c(C.GRAY, f"Span      : {stamped[0]['raw_time']} .. {stamped[-1]['raw_time']}"
                        f"  ({span})"))
    print(c(C.GRAY, f"Parsed    : {datetime.now():%Y-%m-%d %H:%M:%S}"))
    print()

    # Columns that were filtered out of some snapshots, so nobody reads a gap
    # as a collapse to zero.
    partial = []
    for col in columns:
        measured = len(series(rows, col))
        if 0 < measured < len(rows):
            partial.append((col, measured))
    if partial:
        rule("PARTIALLY MEASURED COLUMNS")
        print(c(C.GRAY, "  These were left out of some snapshots by a section filter.\n"
                        "  Gaps are skipped, not read as zero.\n"))
        print_table(
            [{"column": col, "measured": f"{m}/{len(rows)}"} for col, m in partial],
            [("Column", "column", False), ("Measured in", "measured", True)],
        )
    never = [col for col in columns if not series(rows, col)]
    if never:
        print(c(C.GRAY, f"  Never measured: {', '.join(never)}\n"))


def report_runs(runs, analyzed_idx):
    if len(runs) < 2:
        return
    rule("SERVER RESTARTS DETECTED")
    print(c(C.GRAY,
        "  An allocator pool shrank, which cannot happen while one process runs,\n"
        "  so the log spans several server lifetimes. Comparing across a restart\n"
        "  is meaningless, so growth is measured within one run only.\n"))
    table = []
    for i, run in enumerate(runs):
        table.append({
            "run":       f"{i + 1}" + ("  <- analyzed" if i == analyzed_idx else ""),
            "snapshots": len(run),
            "from":      run[0]["raw_time"],
            "to":        run[-1]["raw_time"],
        })
    print_table(table, [
        ("Run",       "run",       False),
        ("Snapshots", "snapshots", True),
        ("From",      "from",      False),
        ("To",        "to",        False),
    ])
    print(c(C.GRAY, "  Use --run N to analyze another run, or --run all to ignore restarts.\n"))


def report_latest(columns, rows):
    acc_cols = accounted_columns(columns)
    # Prefer the newest snapshot that measured every accounted column: on a
    # filtered one the unaccounted figure is mostly the sections nobody asked
    # for, which reads as a 90% leak.
    full = [r for r in rows
            if r.get("ProcessSize") is not None
            and all(r.get(col) is not None for col in acc_cols)]
    last = full[-1] if full else rows[-1]
    partial_only = not full
    process = last.get("ProcessSize")
    measured = [(col, last[col]) for col in acc_cols if last.get(col) is not None]
    measured.sort(key=lambda kv: kv[1], reverse=True)
    accounted = sum(v for _c, v in measured)

    missing = [col for col in acc_cols if last.get(col) is None]

    rule("LATEST SNAPSHOT")
    print(f"  Taken            : {last['raw_time']}")
    if full and last is not rows[-1]:
        print(c(C.GRAY, f"  (the newest fully measured snapshot; {rows[-1]['raw_time']} "
                        f"measured only some sections)"))
    elif partial_only:
        print(c(C.YELLOW, "  (no snapshot measured every section, so the figures below "
                          "are a floor)"))
    print(f"  Process (RSS)    : {human(process)}")
    print(f"  Accounted for    : {human(accounted)}"
          + (f"  ({accounted/process*100:.1f}% of RSS)" if process else ""))
    if process:
        unacc = process - accounted
        col = C.RED if unacc > process * 0.5 else (C.YELLOW if unacc > process * 0.25 else C.GRAY)
        print(c(col, f"  Unaccounted      : {human(unacc)}"
                     f"  ({unacc/process*100:.1f}% of RSS)"))
    print(c(C.GRAY, "  Accounted excludes the allocator pools, which overlap ScriptSize."))
    if missing:
        # Otherwise the unaccounted figure looks alarming when the snapshot was
        # simply asked for less.
        print(c(C.YELLOW,
            f"  This snapshot measured only some sections, so 'unaccounted' is\n"
            f"  overstated by whatever these hold: {', '.join(missing)}"))
    print()

    rule("LARGEST CONSUMERS (latest snapshot)")
    table = []
    for col, val in measured:
        if val == 0:
            continue
        table.append({
            "column": col,
            "size":   human(val),
            "bytes":  num(val),
            "share":  f"{val/accounted*100:.1f}%" if accounted else "-",
        })
    print_table(table, [
        ("Column", "column", False),
        ("Size",   "size",   True),
        ("Bytes",  "bytes",  True),
        ("Share",  "share",  True),
    ])


def report_growth(columns, rows, top):
    if len(rows) < 2:
        print(c(C.GRAY, "  Only one snapshot — nothing to compare yet.\n"))
        return

    stats = [g for g in (growth_row(rows, col) for col in columns) if g]
    movers = [g for g in stats if g["delta"] != 0]
    movers.sort(key=lambda g: abs(g["delta"]), reverse=True)

    rule(f"BIGGEST MOVERS (first vs last measured, top {top})")
    if not movers:
        print(c(C.GREEN, "  Nothing changed across the snapshots.\n"))
        return

    # An hourly rate extrapolated from a few minutes says more about when the
    # snapshots happened to land than about a trend.
    stamped = [r for r in rows if r["time"]]
    if len(stamped) >= 2:
        span_h = hours_between(stamped[0]["time"], stamped[-1]["time"]) or 0
        if span_h < 0.25:
            print(c(C.GRAY,
                f"  Span is only {span_h*60:.0f} minutes — treat the hourly rates as "
                f"indicative at best.\n"))

    table = []
    for g in movers[:top]:
        counts = is_count(g["column"])
        table.append({
            "column": g["column"],
            "first":  num(g["first"]) if counts else human(g["first"]),
            "last":   num(g["last"])  if counts else human(g["last"]),
            "delta":  signed(g["delta"]) if counts else signed_human(g["delta"]),
            "pctc":   pct(g["delta"], g["first"]),
            "rate":   ("-" if g["per_hour"] is None
                       else (f"{g['per_hour']:+,.1f}/h" if counts
                             else signed_human(int(g["per_hour"]), "/h"))),
        })
    print_table(table, [
        ("Column",   "column", False),
        ("First",    "first",  True),
        ("Last",     "last",   True),
        ("Change",   "delta",  True),
        ("%",        "pctc",   True),
        ("Rate",     "rate",   True),
    ])


def report_unaccounted_trend(columns, rows):
    """Is the gap between RSS and what POL can measure itself widening?

    A leak POL accounts for shows up as one column climbing. A leak it cannot
    see shows up only here.
    """
    acc_cols = accounted_columns(columns)
    points = []
    for r in rows:
        if r.get("ProcessSize") is None:
            continue
        vals = [r[col] for col in acc_cols if r.get(col) is not None]
        # Only comparable across snapshots when the same sections were measured.
        if len(vals) != len(acc_cols):
            continue
        points.append((r, r["ProcessSize"] - sum(vals)))

    if len(points) < 2:
        return

    rule("UNACCOUNTED MEMORY TREND")
    first_row, first = points[0]
    last_row,  last  = points[-1]
    delta = last - first
    span = hours_between(first_row["time"], last_row["time"])
    print(f"  First : {human(first)}   ({first_row['raw_time']})")
    print(f"  Last  : {human(last)}   ({last_row['raw_time']})")
    tone = C.RED if delta > 0 else (C.GREEN if delta < 0 else C.GRAY)
    rate = f"  ({human(int(delta/span))}/h)" if span else ""
    print(c(tone, f"  Change: {'+' if delta > 0 else ''}{human(delta)}{rate}"))
    print(c(C.GRAY,
        "  This is process RSS minus everything the core measures. It grows on a\n"
        "  leak the core cannot see, on allocator pools it does not return to the\n"
        "  OS, and on heap fragmentation.\n"))


def report_pairs(columns, rows, top):
    """Split 'more objects' from 'objects getting bigger' — different bugs."""
    ps = pairs(columns)
    if not ps or len(rows) < 2:
        return

    table = []
    for base, ccol, scol in ps:
        cg, sg = growth_row(rows, ccol), growth_row(rows, scol)
        if not cg or not sg or (cg["delta"] == 0 and sg["delta"] == 0):
            continue
        first_avg = (sg["first"] / cg["first"]) if cg["first"] else None
        last_avg  = (sg["last"] / cg["last"]) if cg["last"] else None
        avg_delta = (last_avg - first_avg) if (first_avg is not None and last_avg is not None) else None
        table.append({
            "_sort":  abs(sg["delta"]),
            "base":   base,
            "count":  f"{cg['first']:,} -> {cg['last']:,}",
            "cdelta": signed(cg["delta"]),
            "size":   f"{human(sg['first'])} -> {human(sg['last'])}",
            "avg":    "-" if first_avg is None else f"{first_avg:,.0f} -> {last_avg:,.0f} B",
            "adelta": "-" if avg_delta is None else signed(round(avg_delta), " B"),
        })
    if not table:
        return
    table.sort(key=lambda r: r["_sort"], reverse=True)

    rule("COUNT vs SIZE-PER-ITEM")
    print(c(C.GRAY,
        "  A rising count with a flat average means more things. A flat count\n"
        "  with a rising average means the things themselves are growing.\n"))
    print_table(table[:top], [
        ("Group",         "base",   False),
        ("Count",         "count",  True),
        ("Count Δ",       "cdelta", True),
        ("Total",         "size",   True),
        ("Avg each",      "avg",    True),
        ("Avg Δ",         "adelta", True),
    ])


def report_allocators(columns, rows):
    alloc = [col for col in columns if is_allocator(col)]
    if not alloc:
        return
    table = []
    for col in alloc:
        g = growth_row(rows, col)
        if not g:
            continue
        table.append({
            "column": col,
            "now":    human(g["last"]),
            "delta":  signed_human(g["delta"]),
        })
    if not table:
        return
    rule("ESCRIPT ALLOCATOR POOLS")
    print(c(C.GRAY,
        "  Pool memory reserved for eScript objects. These only ever grow — they\n"
        "  are a high-water mark of churn, not current usage — and they overlap\n"
        "  ScriptSize rather than adding to it, so they are excluded above.\n"))
    print_table(table, [
        ("Pool",   "column", False),
        ("Now",    "now",    True),
        ("Growth", "delta",  True),
    ])


def report_timeline(columns, rows, max_rows=20):
    tracked = [col for col in ("ProcessSize", "ScriptSize", "ObjItemSize", "ObjCharSize")
               if col in columns]
    if not tracked or len(rows) < 2:
        return
    rule("TIMELINE")
    shown = rows if len(rows) <= max_rows else rows[:max_rows // 2] + rows[-(max_rows // 2):]
    table = []
    for i, r in enumerate(shown):
        if len(rows) > max_rows and i == max_rows // 2:
            table.append({"time": "...", **{col: "..." for col in tracked}})
        table.append({"time": r["raw_time"], **{col: human(r.get(col)) for col in tracked}})
    print_table(table, [("Time", "time", False)] + [(col, col, True) for col in tracked])


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    ap = argparse.ArgumentParser(
        description="Analyze POL's memoryusage.log time series."
    )
    ap.add_argument("logfile", nargs="?", default=DEFAULT_LOG, metavar="PATH",
                    help=f"Path to the log (default: {DEFAULT_LOG})")
    ap.add_argument("-n", "--top", type=int, default=15, metavar="N",
                    help="Rows in ranked tables (default: 15)")
    ap.add_argument("--run", default="latest", metavar="N",
                    help="Which server run to measure growth over: 'latest' "
                         "(default), a 1-based number, or 'all' to ignore restarts")
    ap.add_argument("--csv", action="store_true",
                    help="Export the per-column growth table next to the log")
    ap.add_argument("--no-color", action="store_true",
                    help="Never emit ANSI colors (already off when stdout is not a terminal)")
    args = ap.parse_args()

    if args.no_color:
        globals()["USE_COLOR"] = False

    path = Path(args.logfile)
    if not path.exists():
        print(c(C.YELLOW, f"Error: file not found: {path}"))
        print(c(C.GRAY, "  PolCore().log_memory_usage() appends to log/memoryusage.log."))
        raise SystemExit(1)

    try:
        columns, rows = parse_log(path)
    except ValueError as exc:
        print(c(C.YELLOW, f"Error: {exc}"))
        raise SystemExit(1)

    if not rows:
        print(c(C.YELLOW, "No snapshots in the log (header only)."))
        raise SystemExit(0)

    runs = split_runs(columns, rows)
    if args.run == "all":
        window, analyzed_idx = rows, None
    elif args.run == "latest":
        window, analyzed_idx = runs[-1], len(runs) - 1
    else:
        try:
            idx = int(args.run) - 1
            if not 0 <= idx < len(runs):   # runs[-1] is a valid index, not an error
                raise IndexError(args.run)
            window, analyzed_idx = runs[idx], idx
        except (ValueError, IndexError):
            print(c(C.YELLOW, f"Error: --run must be 'latest', 'all', or 1..{len(runs)}"))
            raise SystemExit(1)

    report_overview(path, columns, rows)
    report_runs(runs, analyzed_idx)
    # The latest snapshot is a point in time, so it always comes from the file's
    # end; everything that compares over time uses one run.
    report_latest(columns, rows)
    report_growth(columns, window, args.top)
    report_unaccounted_trend(columns, window)
    report_pairs(columns, window, args.top)
    report_allocators(columns, window)
    report_timeline(columns, rows)

    if args.csv:
        out = path.with_name(path.stem + "_growth.csv")
        stats = [g for g in (growth_row(window, col) for col in columns) if g]
        stats.sort(key=lambda g: abs(g["delta"]), reverse=True)
        with open(out, "w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=["column", "first", "last", "delta",
                                               "samples", "per_hour"])
            w.writeheader()
            w.writerows(stats)
        print(c(C.GREEN, f"  Growth table exported to: {out}\n"))

    print(c(C.CYAN, "=== Analysis complete ===\n"))


if __name__ == "__main__":
    main()
