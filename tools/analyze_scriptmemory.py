#!/usr/bin/env python3
"""
analyze_scriptmemory.py
Parses a POL per-variable memory dump (per-instance Globals/Locals/Stack N
breakdown for every concurrently running instance of one script) and writes
a Markdown or plain-text report.

Input: what PolCore().log_script_variables(script) writes — one timestamped file
per call under log/memory/ (vars-<script>-YYYYMMDD-HHMMSS.log). Older cores, where
this was PolCore().internal(6), appended every dump to a single
log/scriptmemory.log instead; that is accepted too, and so is a directory or a
glob. Each dump becomes its own section in the report, so several dumps of one
script show how it changed over time.

File grammar (as observed; the format has changed between dumps before,
so this parser is deliberately generic rather than hardcoded to one script):

    [MM/DD HH:MM:SS]  <script path>.ecl      <- header, may repeat if the
                                                 file ever holds more than
                                                 one script's dump
    Size: <bytes>                            <- starts a new instance
    Globals
      <name> (<Type>) <bytes>                <- zero or more
    Locals
      <file path>: <line>                    <- current (innermost) frame
      <name> (<Type>) <bytes>                <- zero or more locals there
    Stack <N>                                <- caller frame, N counting
      <file path>: <line>                       down to 0 (outermost)
      <name> (<Type>) <bytes>                <- zero or more locals there
    Stack <N-1>
      ...
    Size: <bytes>                            <- next instance starts here
"""

import re
import sys
import glob as globmod
import argparse
from pathlib import Path
from datetime import datetime
from collections import defaultdict, Counter

# The console summary and report text carry em dashes and similar, which a
# Windows console defaulting to cp1252 cannot encode - it raises rather than
# degrading, killing the run.
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, ValueError):  # not a reconfigurable stream
        pass

# Where log_script_variables() puts its dumps, and the pre-POL100.3.0 file.
SNAPSHOT_DIR  = "memory"
SNAPSHOT_GLOB = "vars-*.log"
LEGACY_LOG    = "scriptmemory.log"


def resolve_inputs(spec):
    """Turn a file, directory or glob into a sorted list of dump files.

    Names carry a YYYYMMDD-HHMMSS stamp, so sorting by name is chronological.
    """
    if spec is None:
        base, candidates = Path("."), (Path(SNAPSHOT_DIR), Path("."))
    else:
        path = Path(spec)
        if path.is_dir():
            # Either the dump directory itself or the log directory holding it,
            # so pointing at log/ works as well as log/memory/.
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

RE_HEADER  = re.compile(r"^\[(\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2})\]\s+(\S.*)$")
# "Size: N" alone, or "Size: N failed to load debug info" when the core could
# not read the .dbg for that instance. Dropping the second form loses the
# instance and its bytes from every total without saying so.
RE_SIZE    = re.compile(r"^Size:\s*(\d+)\s*(.*?)\s*$")
RE_GLOBALS = re.compile(r"^Globals\s*$")
RE_LOCALS  = re.compile(r"^Locals\s*$")
RE_STACK   = re.compile(r"^Stack\s+(\d+)\s*$")
RE_VAR     = re.compile(r"^\s+(\S+)\s+\((\w+)\)\s+(-?\d+)\s*$")
RE_LOC     = re.compile(r"^\s+(.+):\s*(\d+)\s*$")


class Instance:
    __slots__ = ("line_no", "size", "sections", "source", "note")

    def __init__(self, line_no, size, source=None, note=""):
        self.line_no = line_no
        self.size = size
        self.source = source  # file it came from, when several were read
        self.note = note      # e.g. "failed to load debug info" - no breakdown
        # list of (section_label, location_or_None, [(name, type, bytes), ...])
        self.sections = []

    def all_vars(self):
        for _label, _loc, vars_ in self.sections:
            for v in vars_:
                yield v

    def itemized_bytes(self):
        return sum(b for _n, _t, b in self.all_vars())

    def var_map(self):
        # last-write-wins if a name somehow repeats within one instance
        return {n: (t, b) for n, t, b in self.all_vars()}

    def chain(self):
        parts = []
        for label, loc, _vars in self.sections:
            if loc:
                parts.append(f"{loc[0]}:{loc[1]}")
        return " <- ".join(parts) if parts else "(no call-stack recorded)"


def parse(path, groups=None, label_source=False):
    """Parse one file into {(timestamp, script): [Instance, ...]}.

    Pass an existing `groups` to accumulate across several files; a dump of the
    same script at a different timestamp keys separately, so each snapshot
    stays its own group.
    """
    groups = defaultdict(list) if groups is None else groups
    source = path.name if label_source else None
    current_header = None
    current = None  # Instance being built
    section_label = None
    section_loc = None
    section_vars = None

    def flush_section():
        if current is not None and section_label is not None:
            current.sections.append((section_label, section_loc, section_vars))

    def flush_instance():
        nonlocal current
        flush_section()
        if current is not None:
            # A dump with no header of its own still has to land somewhere; the
            # key is unpacked as (timestamp, script) downstream.
            key = current_header or ("?", path.name)
            groups[key].append(current)
        current = None

    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for line_no, raw in enumerate(fh, 1):
            line = raw.rstrip("\n\r")
            if not line.strip():
                continue

            m = RE_HEADER.match(line)
            if m:
                flush_instance()
                current_header = (m.group(1), m.group(2))
                section_label = section_loc = section_vars = None
                continue

            m = RE_SIZE.match(line)
            if m:
                flush_instance()
                current = Instance(line_no, int(m.group(1)), source, m.group(2))
                section_label = section_loc = section_vars = None
                continue

            if current is None:
                continue  # stray line before first Size:/header

            if RE_GLOBALS.match(line):
                flush_section()
                section_label, section_loc, section_vars = "Globals", None, []
                continue

            if RE_LOCALS.match(line):
                flush_section()
                section_label, section_loc, section_vars = "Locals", None, []
                continue

            m = RE_STACK.match(line)
            if m:
                flush_section()
                section_label, section_loc, section_vars = f"Stack {m.group(1)}", None, []
                continue

            m = RE_VAR.match(line)
            if m and section_vars is not None:
                section_vars.append((m.group(1), m.group(2), int(m.group(3))))
                continue

            m = RE_LOC.match(line)
            if m and section_loc is None:
                section_loc = (m.group(1).strip(), int(m.group(2)))
                continue

        flush_instance()

    return groups


def fmt_bytes(n):
    return f"{n:,} B"


def fmt_mb(n):
    return f"{n / 1024 / 1024:.2f} MB"


def build_report(script_name, timestamp, instances, top_n, max_anomalies):
    lines = []
    W = lines.append

    n = len(instances)
    sizes = [i.size for i in instances]
    size_counts = Counter(sizes)
    mode_size, mode_count = size_counts.most_common(1)[0]
    total_size = sum(sizes)

    W(f"# Script Memory Report — `{script_name}`")
    W("")
    W(f"- **Snapshot timestamp:** {timestamp}")
    W(f"- **Generated:** {datetime.now():%Y-%m-%d %H:%M:%S}")
    W(f"- **Instances parsed:** {n:,}")
    W("")

    # ---------------- Summary ----------------
    W("## Summary")
    W("")
    W(f"- Total reported size: **{fmt_bytes(total_size)}** ({fmt_mb(total_size)})")
    W(f"- Average instance size: {fmt_bytes(round(total_size / n))}")
    W(f"- Min / Max instance size: {fmt_bytes(min(sizes))} / {fmt_bytes(max(sizes))}")
    W(f"- Distinct size values: {len(size_counts)}")
    W(f"- Modal size (most common): {fmt_bytes(mode_size)} — {mode_count:,} instances ({mode_count/n*100:.1f}%)")
    noted = [i for i in instances if i.note]
    if noted:
        # These still count towards the totals above - the core reported a size
        # for them, just no variables - so say how much of the report is blind.
        blind = sum(i.size for i in noted)
        reasons = ", ".join(sorted({i.note for i in noted}))
        W(f"- **{len(noted):,} instance(s) with no variable breakdown** "
          f"({fmt_bytes(blind)}, {blind/total_size*100:.1f}% of the total): {reasons}")
    W("")

    # ---------------- Size distribution ----------------
    W("## Size distribution")
    W("")
    W("| Size | Instances | Share | Δ vs mode |")
    W("|---|---:|---:|---:|")
    for size, count in size_counts.most_common(top_n):
        delta = size - mode_size
        delta_s = f"+{delta:,} B" if delta > 0 else (f"{delta:,} B" if delta < 0 else "—")
        W(f"| {size:,} B | {count:,} | {count/n*100:.1f}% | {delta_s} |")
    remaining = len(size_counts) - top_n
    if remaining > 0:
        shown = sum(c for _s, c in size_counts.most_common(top_n))
        W(f"| *…{remaining} more distinct sizes…* | {n - shown:,} | {(n-shown)/n*100:.1f}% | |")
    W("")

    # ---------------- Suspend points / call chains ----------------
    chain_counts = Counter(i.chain() for i in instances)
    W("## Suspend points (call-stack chains)")
    W("")
    W("Where the current/innermost frame down to the outermost frame was paused")
    W("when the dump was taken (innermost first).")
    W("")
    W("| Call chain | Instances | Share |")
    W("|---|---:|---:|")
    for chain, count in chain_counts.most_common(top_n):
        W(f"| `{chain}` | {count:,} | {count/n*100:.1f}% |")
    W("")

    # ---------------- Variables ----------------
    var_section = {}   # name -> most common section label
    var_types = defaultdict(Counter)
    var_bytes = defaultdict(list)
    var_count = Counter()
    first_line_for_size = {}

    for inst in instances:
        first_line_for_size.setdefault(inst.size, inst.line_no)
        for label, _loc, vars_ in inst.sections:
            for name, typ, b in vars_:
                var_count[name] += 1
                var_types[name][typ] += 1
                var_bytes[name].append(b)
                var_section.setdefault(name, label)

    def avg(lst):
        return sum(lst) / len(lst)

    rows = []
    for name in var_count:
        blist = var_bytes[name]
        types = var_types[name]
        variant_type = len(types) > 1
        variant_bytes = len(set(blist)) > 1
        rows.append({
            "name": name,
            "section": var_section[name],
            "types": types,
            "count": var_count[name],
            "avg": avg(blist),
            "min": min(blist),
            "max": max(blist),
            "variant_type": variant_type,
            "variant_bytes": variant_bytes,
        })
    rows.sort(key=lambda r: r["avg"], reverse=True)

    W("## Variables (Globals + Locals across all frames)")
    W("")
    W("Sorted by average byte size, largest first. `min`/`max` differing from")
    W("`avg` means the value's size (or type) is not constant across instances.")
    W("")
    W("| Variable | Section | Type(s) | Seen in | Avg | Min | Max | Notes |")
    W("|---|---|---|---:|---:|---:|---:|---|")
    for r in rows:
        type_s = ", ".join(f"{t}×{c}" for t, c in r["types"].most_common()) if r["variant_type"] else next(iter(r["types"]))
        notes = []
        if r["variant_type"]:
            notes.append("type varies")
        if r["variant_bytes"]:
            notes.append("size varies")
        notes_s = ", ".join(notes) if notes else ""
        W(f"| `{r['name']}` | {r['section']} | {type_s} | {r['count']:,}/{n:,} | {r['avg']:,.0f} B | {r['min']:,} B | {r['max']:,} B | {notes_s} |")
    W("")

    # ---------------- Notable / type-variant variables ----------------
    variant_rows = [r for r in rows if r["variant_type"] or r["variant_bytes"]]
    if variant_rows:
        W("## Notable variables (type or size varies across instances)")
        W("")
        for r in variant_rows:
            W(f"- **`{r['name']}`** ({r['section']}): " +
              ", ".join(f"{t} in {c:,} instance(s)" for t, c in r["types"].most_common()) +
              f" — sizes range {r['min']:,}-{r['max']:,} B")
        W("")

    # ---------------- Hidden overhead ----------------
    sig_to_sizes = defaultdict(Counter)
    for inst in instances:
        if inst.note:       # no variables to match on; every one would collide
            continue
        sig = tuple(sorted(inst.var_map().items()))
        sig_to_sizes[sig][inst.size] += 1

    multi_size_sigs = {sig: sizes for sig, sizes in sig_to_sizes.items() if len(sizes) > 1}
    # An instance with no breakdown itemizes nothing, so its "overhead" is its
    # whole size - averaging that in would drown the figure this measures.
    itemizable = [i for i in instances if not i.note]
    overheads = [inst.size - inst.itemized_bytes() for inst in itemizable]

    W("## Unattributed size (overhead not itemized by any variable)")
    W("")
    if not overheads:
        W("- No instance reported a variable breakdown, so there is nothing to compare.")
    else:
        W(f"- Average unattributed bytes per instance: {avg(overheads):.0f} B")
        W(f"- Range: {min(overheads):,} B – {max(overheads):,} B")
        if len(itemizable) != n:
            W(f"- Excludes {n - len(itemizable):,} instance(s) with no breakdown.")
    if multi_size_sigs:
        affected = sum(sum(sizes.values()) for sizes in multi_size_sigs.values())
        W("")
        W(f"- **{len(multi_size_sigs):,} distinct instance(s)-with-identical-variables group(s)** "
          f"(covering {affected:,} instances) report **more than one `Size:` total** for the exact "
          f"same set of variable names, types and byte values. The reported total is not fully "
          f"explained by the itemized breakdown — treat `Size:` as authoritative and the variable "
          f"list as a partial account.")
        W("")
        W("| Example itemized content | Sizes seen |")
        W("|---|---|")
        for sig, sizes in list(multi_size_sigs.items())[:min(5, len(multi_size_sigs))]:
            sizes_s = ", ".join(f"{s:,} B ×{c:,}" for s, c in sorted(sizes.items()))
            W(f"| {len(sig)} variables, {sum(b for _n,(_t,b) in sig):,} B itemized | {sizes_s} |")
    else:
        W("- No two instances with identical itemized variables reported different totals.")
    W("")

    # ---------------- Anomalous instances ----------------
    anomalies = [i for i in instances if i.size != mode_size]
    W("## Anomalous instances (size differs from the mode)")
    W("")
    if not anomalies:
        W("None — every instance matches the modal size.")
    else:
        W(f"{len(anomalies):,} of {n:,} instances ({len(anomalies)/n*100:.1f}%) differ from the "
          f"modal size of {mode_size:,} B. Showing up to {max_anomalies}, diffed against the first "
          f"modal-size instance (file line {first_line_for_size.get(mode_size, '?')}).")
        W("")
        reference = next((i for i in instances if i.size == mode_size), None)
        ref_map = reference.var_map() if reference else {}

        # Guarantee at least one example per distinct anomalous size (most
        # frequent sizes first) instead of exhausting the budget on whichever
        # anomaly happens to appear first in the file.
        by_size = defaultdict(list)
        for inst in anomalies:
            by_size[inst.size].append(inst)
        distinct_sizes = sorted(by_size, key=lambda s: len(by_size[s]), reverse=True)
        per_size_budget = max(1, max_anomalies // len(distinct_sizes))

        examples = []
        for size in distinct_sizes:
            examples.extend(by_size[size][:per_size_budget])
        examples = examples[:max_anomalies]
        examples.sort(key=lambda i: i.line_no)

        for inst in examples:
            if inst.note:
                where = f"{inst.source}:{inst.line_no}" if inst.source else f"Line {inst.line_no}"
                delta = inst.size - mode_size
                W(f"- {where}: size {inst.size:,} B ({delta:+,} B) — {inst.note}")
                continue
            cur_map = inst.var_map()
            diffs = []
            for name, (t, b) in cur_map.items():
                if name not in ref_map:
                    diffs.append(f"`{name}` added ({t}, {b:,} B)")
                elif ref_map[name] != (t, b):
                    rt, rb = ref_map[name]
                    diffs.append(f"`{name}`: {rt}({rb:,} B) -> {t}({b:,} B)")
            for name in ref_map:
                if name not in cur_map:
                    diffs.append(f"`{name}` missing")
            delta = inst.size - mode_size
            delta_s = f"+{delta:,} B" if delta > 0 else f"{delta:,} B"
            diff_s = "; ".join(diffs) if diffs else "(no itemized differences found — see Unattributed section)"
            where = f"{inst.source}:{inst.line_no}" if inst.source else f"Line {inst.line_no}"
            W(f"- {where}: size {inst.size:,} B ({delta_s}) — {diff_s}")
    W("")

    return "\n".join(lines)


def _clean(text):
    return text.replace("**", "").replace("`", "")


def build_report_txt(md_text):
    """Markdown -> plain-text: headers become underlined labels, tables get
    their columns measured and padded instead of being left as raw pipes."""
    out = []
    md_lines = md_text.splitlines()
    i = 0
    while i < len(md_lines):
        line = md_lines[i]
        if line.startswith("#"):
            text = _clean(line.lstrip("#").strip())
            out.append(text)
            out.append("-" * len(text))
            i += 1
            continue
        if line.startswith("|"):
            # collect the whole table block (header, separator, all rows)
            table_lines = []
            while i < len(md_lines) and md_lines[i].startswith("|"):
                table_lines.append(md_lines[i])
                i += 1
            rows = []
            for tl in table_lines:
                cells = [_clean(c.strip()) for c in tl.strip("|").split("|")]
                if set("".join(cells)) <= set("-: "):
                    continue  # the |---|---| separator row
                rows.append(cells)
            if rows:
                widths = [max(len(r[c]) for r in rows) for c in range(len(rows[0]))]
                for r_idx, r in enumerate(rows):
                    padded = [cell.ljust(widths[c]) for c, cell in enumerate(r)]
                    out.append("  " + "  ".join(padded))
                    if r_idx == 0:
                        out.append("  " + "  ".join("-" * w for w in widths))
            continue
        out.append(_clean(line))
        i += 1
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser(
        description="Parse POL per-variable memory dumps and generate a report."
    )
    ap.add_argument(
        "logfile", nargs="?", default=None, metavar="PATH",
        help=f"Dump file, directory or glob. Default: {SNAPSHOT_DIR}/{SNAPSHOT_GLOB}, "
             f"falling back to ./{SNAPSHOT_GLOB} then the legacy {LEGACY_LOG}.",
    )
    ap.add_argument("-o", "--output", help="Report output path (default: <logfile>_report.<ext>)")
    ap.add_argument("--format", choices=["md", "txt"], default="md", help="Report format (default: md)")
    ap.add_argument("--top", type=int, default=15, metavar="N", help="Rows to show in ranked tables (default 15)")
    ap.add_argument("--max-anomalies", type=int, default=20, metavar="N", help="Max anomalous instances to list")
    ap.add_argument("--quiet", action="store_true", help="Suppress console summary")
    args = ap.parse_args()

    log_paths = resolve_inputs(args.logfile)
    if not log_paths:
        where = args.logfile or f"{SNAPSHOT_DIR}/{SNAPSHOT_GLOB} (or ./{LEGACY_LOG})"
        print(f"Error: no dump files found: {where}", file=sys.stderr)
        print("  PolCore().log_script_variables(script) writes them under log/memory/.",
              file=sys.stderr)
        raise SystemExit(1)

    groups = defaultdict(list)
    for path in log_paths:
        parse(path, groups, label_source=len(log_paths) > 1)
    if not groups:
        print("No instances found (no 'Size:' lines matched).", file=sys.stderr)
        raise SystemExit(1)

    if args.output:
        out_path = Path(args.output)
    elif len(log_paths) == 1:
        out_path = log_paths[0].with_name(log_paths[0].stem + "_report." + args.format)
    else:
        # One report covering several dumps is named for the set, not for
        # whichever file happened to be first.
        out_path = log_paths[0].parent / f"vars-combined_report.{args.format}"

    reports_md = []
    for (timestamp, script), instances in groups.items():
        reports_md.append(build_report(script, timestamp, instances, args.top, args.max_anomalies))
    full_md = "\n\n---\n\n".join(reports_md)

    content = full_md if args.format == "md" else build_report_txt(full_md)
    out_path.write_text(content, encoding="utf-8")

    if not args.quiet:
        total_instances = sum(len(v) for v in groups.values())
        print(f"Parsed {total_instances:,} instance(s) across {len(groups)} script dump(s).")
        print(f"Report written to: {out_path}")


if __name__ == "__main__":
    main()
