"""Runs POL as the last stage of the shard test pipeline, and says when it has exited.

cmake/core_tests_start.cmake runs testclient.py, deafclient.py, rawpeer.py and POL as one
execute_process pipeline, and cmake waits for every stage. The helpers learn that the shard
is up and gone by probing its ports, so a POL that stops while loading its configuration,
before it listens on anything, looks to them like a shard that has not started yet. They
then wait out their whole deadline for it.

This process starts POL with the same stdin and stdout, waits for it, and then writes
EXITED_MARKER in the working directory, holding the pid of the cmake process that started
the pipeline. Every stage is that process's child, so a helper stops waiting once the marker
names its own parent, and a marker left by another run names someone else. Its exit code is
POL's.
"""

import os
import subprocess
import sys

# read by pol_exited() in testclient.py, deafclient.py and rawpeer.py
EXITED_MARKER = "pol.exited"


def main():
    try:
        os.remove(EXITED_MARKER)
    except FileNotFoundError:
        pass
    try:
        code = subprocess.call(sys.argv[1:])
    finally:
        with open(EXITED_MARKER, "w", encoding="utf-8") as marker:
            marker.write(f"{os.getppid()}\n")
    return code


if __name__ == "__main__":
    sys.exit(main())
