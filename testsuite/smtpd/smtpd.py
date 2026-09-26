import asyncio
import ssl
import os
import sys
import subprocess
from aiosmtpd.controller import Controller
from aiosmtpd.handlers import AsyncMessage
from aiosmtpd.smtp import AuthResult, LoginPassword, SMTP, syntax
from aiosmtpd.handlers import Debugging

def authenticator_func(server, session, envelope, mechanism, auth_data):
    return AuthResult(success=True)

# Load SSL context
context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
context.load_cert_chain('cert.pem', 'key.pem')

class MyServer(SMTP):
    @syntax('SHUTDOWN')
    async def smtp_SHUTDOWN(self, arg):
        print("Server shutdown requested")
        os._exit(0)

class ControllerStarttls(Controller):
    def factory(self):
        return MyServer(self.handler, require_starttls=False, tls_context=context, authenticator=authenticator_func, auth_required=True)

def pol_exited():
    """True once POL has exited: testsuite/runpol.py writes pol.exited when it does.

    Only the email test sends SHUTDOWN, so a shard that stops without running it would
    otherwise keep this stage of the pipeline open. The marker holds the pid of the cmake
    process running the pipeline, this process's parent too; one naming another parent is
    left over from an earlier run.
    """
    try:
        with open("pol.exited", encoding="utf-8") as marker:
            return int(marker.read().strip() or 0) == os.getppid()
    except (OSError, ValueError):
        return False

def exit_with_pol(loop):
    if pol_exited():
        print("POL exited, stopping the SMTP server")
        loop.stop()
        return
    loop.call_later(1.0, exit_with_pol, loop)

testfilter = os.environ.get('POLCORE_TEST_FILTER', '')

if not testfilter or testfilter.lower().startswith('testemail'):
    with open("smtpd.log", "w", encoding="utf-8", buffering=1) as output_file:
        controller = ControllerStarttls(Debugging(stream = output_file), hostname='localhost', port=1025)
        controller.start()

        print("SMTP server running on smtp://localhost:1025 (CTRL+C to stop, or send 'SHUTDOWN' command)")

        try:
            loop = asyncio.new_event_loop()
            loop.call_soon(exit_with_pol, loop)
            loop.run_forever()
        except KeyboardInterrupt:
            pass
        finally:
            controller.stop()
