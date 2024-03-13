import asyncio
from pathlib import Path
import subprocess
import sys
from typing import Awaitable
import traceback

# seconds; error if compilation doesn't occur within this duration
ECOMPILE_WATCH_TIMEOUT=5

def print(*args, **kwargs):
    __builtins__.print(*args, flush=True, **kwargs)

if len(sys.argv) <= 1:
    print("Must pass ecompile binary as first argument.")
    exit(1)

ECOMPILE_BIN = sys.argv[1]

class EcompileExecutor:
    _compilation_future = None
    _process = None

    async def _read_output(self, stream, started_future):
        in_compilation_summary = False
        compilation_summary = {
            'up_to_date': 0,
            'compiled': 0,
            'errored': 0,
        }

        while True:
            line = await stream.readline()
            if not line:
                break

            line = line.decode().strip()
            print("Output from subprocess:", line)

            if "Compilation Summary:" in line:
                in_compilation_summary = True

            elif line == "" and in_compilation_summary:
                in_compilation_summary = False
                if self._compilation_future is not None:
                    self._compilation_future.set_result(compilation_summary)
                compilation_summary = {
                    'up_to_date': 0,
                    'compiled': 0,
                    'errored': 0,
                }

            elif "Compiled " in line:
                compilation_summary["compiled"] = int(line[line.index(' ')+1:line.index(' script')])

            elif "scripts were already up-to-date" in line:
                compilation_summary["up_to_date"] = int(line[0:line.index(' ')])

            elif "of those script" in line:
                compilation_summary["errored"] = int(line[0:line.index(' ')])

            elif started_future and "Entering watch mode." in line:
                started_future.set_result(True)

    async def start(self):
        command = [ECOMPILE_BIN, '-Au', '-W']
        self._process = await asyncio.create_subprocess_exec(
            *command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        started_future = asyncio.Future()

        asyncio.create_task(self._read_output(self._process.stdout, started_future))

        try:
            await asyncio.wait_for(started_future, timeout=ECOMPILE_WATCH_TIMEOUT)
        except asyncio.TimeoutError:
            print(f"ecompile watch mode did not start after {ECOMPILE_WATCH_TIMEOUT} seconds")

    def new_compliation_future(self) -> Awaitable[dict[str, int]]:
        self._compilation_future = asyncio.Future()

        return self._compilation_future

    async def stop(self):
        self._process.kill()
        await self._process.wait()


# Apparently, we need to write _and_ touch the file for proper testing
# functionality across win32, darwin, and linux...?
def update_file(filename, text = None):
    path = Path(filename)
    old_text = None
    with open(filename, 'r+' if path.is_file() else 'w+') as f:
        old_text = f.read()
        f.seek(0)
        f.write(old_text if text is None else text)
        f.truncate()
    Path(filename).touch()
    return old_text

def Test(name):
    def decorator(func):
        def wrapper(*args, **kwargs):
            result = func(*args, **kwargs)
            return result
        wrapper.test_name = name
        return wrapper
    return decorator

class TestRunner:
    executor = None

    def __init__(self):
        self.executor = EcompileExecutor()

    async def run_tests(self):
        # This script gets created in the "new script" test.
        # Remvove it (if exists) before starting ecompile watch.
        new_script_path = Path("scripts/new_source_file.src")
        if new_script_path.is_file():
            new_script_path.unlink()

        await self.executor.start()

        print("Entered watch mode. Starting tests...")

        test_methods = [
            getattr(self, name)
            for name in dir(self)
            if callable(getattr(self, name))
            and not name.startswith("_")
            and hasattr(getattr(self, name), "test_name")
        ]

        failed_tests = []
        def report_failure(msg):
            print(msg)
            failed_tests.append(msg)

        for func in test_methods:
            try:
                print(f"Running test: {func.test_name}")
                await func()
                print(f"Test passing: {func.test_name}")
            except asyncio.TimeoutError:
                report_failure(f"Test failing: {func.test_name}: Timed out")
            except Exception as e:
                report_failure(f"Test failing: {func.test_name}: {e}")
                if type(e) != AssertionError:
                    traceback.print_exception(e)

        await self.executor.stop()

        return failed_tests

    @Test("Editing a script compiles only that script")
    async def test01(self):
        compliation_future = self.executor.new_compliation_future()

        update_file("scripts/start.src")

        result = await asyncio.wait_for(compliation_future, timeout=ECOMPILE_WATCH_TIMEOUT)
        assert result['compiled'] == 1, f"Number of compiled scripts {result['compiled']} != 1"

    @Test("Editing an include compiles all scripts dependent on that include")
    async def test02(self):
        compliation_future = self.executor.new_compliation_future()

        update_file("scripts/include/sysevent.inc")

        result = await asyncio.wait_for(compliation_future, timeout=ECOMPILE_WATCH_TIMEOUT)
        assert result['compiled'] > 0, f"Number of compiled scripts {result['compiled']} <= 0"

    @Test("Creating a new script compiles that script")
    async def test03(self):
        compliation_future = self.executor.new_compliation_future()

        update_file("scripts/new_source_file.src", "")

        try:
            result = await asyncio.wait_for(compliation_future, timeout=ECOMPILE_WATCH_TIMEOUT)
            assert result['compiled'] == 1, f"Number of compiled scripts {result['compiled']} != 1"
        finally:
            Path("scripts/new_source_file.src").unlink()

    @Test("Editing an include to fail will successfully recompile sources when include is fixed")
    async def test04(self):
        compliation_future = self.executor.new_compliation_future()
        old_text = update_file("scripts/include/sysevent.inc", "foo")
        try:
            result = await asyncio.wait_for(compliation_future, timeout=ECOMPILE_WATCH_TIMEOUT)
            assert result['compiled'] > 0, f"Number of compiled scripts with errored include {result['compiled']} <= 0"
            assert result['errored'] > 0, f"Number of errored scripts with errored include {result['errored']} <= 0"

            compliation_future = self.executor.new_compliation_future()

            old_text = update_file("scripts/include/sysevent.inc", old_text)
            result = await asyncio.wait_for(compliation_future, timeout=ECOMPILE_WATCH_TIMEOUT)
            assert result['compiled'] > 0, f"Number of compiled scripts with fixed include {result['compiled']} <= 0"
            assert result['errored'] == 0, f"Number of errored scripts with fixed include {result['errored']} != 0"
        except Exception as e:
            update_file("scripts/include/sysevent.inc", old_text)
            raise e

async def main():
    runner = TestRunner()
    failed_tests = await runner.run_tests()
    if len(failed_tests) > 0:
        print("Tests failed!")
        for failed_test in failed_tests:
            print(f" - {failed_test}")
        exit(1)
    else:
        print("All tests passed!")

asyncio.run(main())
