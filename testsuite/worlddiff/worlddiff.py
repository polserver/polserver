#!/usr/bin/env python3
"""Compare two POL world saves after a normalising pass.

Used by the ``shard_save_roundtrip`` ctest: the shard loads a world and saves it again
without touching it, and this script asserts the second save carries the same data as the
first.  A raw byte comparison cannot be used - see NORMALISATION below.

    worlddiff.py <dir_a> <dir_b> [--verbose]

Exit code 0 if every data file matches, 1 otherwise (with a diff on stdout).
"""

import argparse
import difflib
import os
import sys

# NORMALISATION
#
# Three things differ between two saves of the same world for reasons that have nothing to
# do with what was persisted, so they are normalised away:
#
# 1. Block order, but only in OBJECTHASH_ORDERED below.  Every other file is compared in
#    file order - see BLOCK ORDER.
#
# 2. The comment header, which carries the core version and object counts.
#
# 3. VOLATILE_KEYS and VOLATILE_PROPS below - values that are not a function of the world
#    contents, so two saves of the same world may legitimately disagree about them.
#
# Everything else is compared verbatim, including the order of keys inside a block: the
# writers emit those in a fixed code order, so a change there is a real change.

# BLOCK ORDER
#
# Most of these files are compared in file order, because for them the order means
# something. items.txt and multis.txt are written by walking realm zones
# (savedata.cpp: write_items/write_multis), and the load reads them back in file order into
# the same vectors via add_item_to_world's zone.items.push_back - so file order IS zone
# vector order, which every WorldIterator walk hands to scripts as the result order of
# ListItemsNearLocation and friends. specs/items/08 reworks exactly those insert/erase
# helpers, so this file is the one place that would notice it reordering them.
#
# The exception is the files written by walking objStorageManager.objecthash, an
# unordered_map whose iteration order is an STL implementation detail. It was in fact stable
# across every round trip measured here, but that is not a property worth asserting across
# platforms and standard libraries, so those files are sorted by a stable key instead.
#
# Known soft spot, verified rather than assumed: ONE on-cursor item in the starting save is
# enough to fail this test, legitimately. write_items appends gotten items after its zone
# walk, and a lifted item has been extricated from its zone, so it exists only in that tail.
# Reloading it makes it an ordinary ground item at the holder's feet, and the next save
# writes it in zone order - same bytes, different place in the file. Confirmed by moving a
# block to the tail by hand and round-tripping it: the reload put it back in zone order and
# this comparison reported it.
#
# It is unreachable as the tests are wired today, and the reason is worth knowing before
# anyone rewires them: the round trip starts from what shard_test_2 saved, and shard_test_2
# is a bare `pol` with POLCORE_TEST_RUN=2 and no pyuo client, so it cannot put an item on a
# cursor. Whatever pass 1 left on a cursor was already converted to a ground item when
# shard_test_2 loaded it. Giving pass 2 a client, or re-pointing this test at shard_test_1's
# save, makes the hazard live.
#
# If that happens, do not sort items.txt - that would give up the zone-order signal for the
# whole file to accommodate one block. Mark the item instead: set a CProp on it before the
# save and exempt blocks carrying that CProp from position comparison here, matching them by
# content as a set while every other block stays in file order. The contents do line up
# across the two saves (WriteGottenItem writes the holder's coordinates, which is exactly
# where the reloaded ground item sits), so a per-object exemption is enough.
OBJECTHASH_ORDERED = {
    "pcs.txt",
    "pcequip.txt",
    "npcs.txt",
    "npcequip.txt",
}

# Keys dropped from every block before comparing.
VOLATILE_KEYS = {
    # The serial allocators are boot history, not world data. GetNextUnusedItemSerial()
    # scans upward from the current counter and skips serials already in the objecthash,
    # while read_data() clamps the counter back down to the stored value only when the
    # stored value is the smaller of the two (uimport.cpp, dave's 3/9/3 comment). So the
    # counter ratchets past whatever live serials sit above it: measured on the test shard,
    # the first reload after a test pass advanced LastItemSerialNumber by 14, the next by 1,
    # and every reload after that by 0, on a world whose files were byte-identical
    # throughout. Comparing them would make this test fail on the run that matters - the one
    # straight after the test passes - and pass on reruns.
    "LastItemSerialNumber",
    "LastCharSerialNumber",
}

# CProp names dropped from every block before comparing.
VOLATILE_PROPS = {
    "gameclock",  # advances between the load and the save
}

# The world data directory also holds .bak (previous save) and .ndt (save in progress)
# copies of each file, and the ds/ datastore directory. Only the .txt files are compared.


class Block:
    r"""One `Type [Header]\n{\n\tKey\tValue\n...\n}` element."""

    def __init__(self, type_line, lines):
        self.type_line = type_line
        self.lines = lines

    def sort_key(self):
        # Serial identifies items/characters; Name identifies storage areas and the like.
        # The full body is the final tiebreak so the order is total and stable.
        keyed = dict()
        for key, value in self.lines:
            keyed.setdefault(key, value)
        return (
            self.type_line,
            keyed.get("Serial", ""),
            keyed.get("Name", ""),
            tuple(self.lines),
        )

    def render(self):
        out = [self.type_line, "{"]
        out.extend("\t%s\t%s" % (key, value) for key, value in self.lines)
        out.append("}")
        return out


def parse(path):
    """Parse a world data file into normalised blocks. Raises on malformed input."""
    blocks = []
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
        lines = f.read().splitlines()

    i = 0
    while i < len(lines):
        line = lines[i]
        if not line.strip() or line.startswith("#"):
            i += 1
            continue
        type_line = line
        i += 1
        if i >= len(lines) or lines[i] != "{":
            raise ValueError("%s:%d: expected '{' after %r" % (path, i + 1, type_line))
        i += 1
        body = []
        while i < len(lines) and lines[i] != "}":
            entry = lines[i]
            i += 1
            # blocks may carry comments of their own - resource.txt annotates each pool
            # with the config file it came from
            if entry.startswith("#") or not entry.strip():
                continue
            if not entry.startswith("\t"):
                raise ValueError("%s:%d: expected a key line, got %r" % (path, i, entry))
            key, _, value = entry[1:].partition("\t")
            if key in VOLATILE_KEYS:
                continue
            if key == "CProp" and value.split(" ", 1)[0] in VOLATILE_PROPS:
                continue
            body.append((key, value))
        if i >= len(lines):
            raise ValueError("%s: unterminated block %r" % (path, type_line))
        i += 1  # the '}'
        blocks.append(Block(type_line, body))
    return blocks


def canonical(path):
    blocks = parse(path)
    if os.path.basename(path) in OBJECTHASH_ORDERED:
        blocks.sort(key=Block.sort_key)
    out = []
    for block in blocks:
        out.extend(block.render())
    return out


def data_files(directory):
    return sorted(name for name in os.listdir(directory) if name.endswith(".txt"))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("dir_a", help="the save the round trip started from")
    ap.add_argument("dir_b", help="the save the shard wrote after loading it")
    ap.add_argument("--verbose", action="store_true", help="report matching files too")
    ap.add_argument(
        "--context", type=int, default=3, help="diff context lines (default 3)"
    )
    args = ap.parse_args()

    files_a = data_files(args.dir_a)
    files_b = data_files(args.dir_b)
    failed = False

    only_a = set(files_a) - set(files_b)
    only_b = set(files_b) - set(files_a)
    for name in sorted(only_a):
        print("MISSING: %s was saved before the round trip but not after" % name)
        failed = True
    for name in sorted(only_b):
        print("EXTRA:   %s appeared during the round trip" % name)
        failed = True

    for name in files_a:
        if name in only_a:
            continue
        path_a = os.path.join(args.dir_a, name)
        path_b = os.path.join(args.dir_b, name)
        try:
            canon_a = canonical(path_a)
            canon_b = canonical(path_b)
        except ValueError as error:
            print("PARSE:   %s" % error)
            failed = True
            continue
        if canon_a == canon_b:
            if args.verbose:
                print("ok:      %s (%d blocks)" % (name, canon_a.count("{")))
            continue
        failed = True
        print("DIFFERS: %s" % name)
        diff = difflib.unified_diff(
            canon_a,
            canon_b,
            fromfile="%s (before)" % name,
            tofile="%s (after)" % name,
            n=args.context,
            lineterm="",
        )
        for line in diff:
            print("  " + line)

    if failed:
        print("world save round trip FAILED")
        return 1
    print("world save round trip ok: %d files match" % len(files_a))
    return 0


if __name__ == "__main__":
    sys.exit(main())
