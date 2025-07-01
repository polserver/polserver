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
    print("=== Authentication Attempt ===")
    print(f"Mechanism: {mechanism}")
    print(f"Auth Data: {auth_data}")
    return AuthResult(success=True)

# Load SSL context
context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
context.load_cert_chain('cert.pem', 'key.pem')

class MyServer(SMTP):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @syntax('SHUTDOWN')
    async def smtp_SHUTDOWN(self, arg):
        print("Server shutdown requested")
        os._exit(0)

class ControllerStarttls(Controller):
    def factory(self):
        return MyServer(self.handler, require_starttls=False, tls_context=context, authenticator=authenticator_func, auth_required=True)

with open("smtpd.log", "w", encoding="utf-8", buffering=1) as output_file:
    controller = ControllerStarttls(Debugging(stream = output_file), hostname='localhost', port=1025)
    controller.start()

    print("SMTP server running on smtp://localhost:1025 (CTRL+C to stop, or send 'SHUTDOWN' command)")

    try:
        asyncio.get_event_loop().run_forever()
    except KeyboardInterrupt:
        pass
    finally:
        controller.stop()
