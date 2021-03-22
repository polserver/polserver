[![Build Status](https://travis-ci.org/gtozzi/pyuo.svg?branch=master)](https://travis-ci.org/gtozzi/pyuo)

# pyuo
A text-only UO client written in Python
Copyright (C) 2015-2016 Gabriele Tozzi

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

## System Requirements
- Python 3.4+

## Archive content
Examples:
- *spary.py* is a very simple draft of a script using this client
- *terminal.py* is curses-based interactive command line client

The library itself is contained in the *pyuo* folder:
- *brain.py* contains the classes useful for writing your scripts
- *client.py* contains the client classes

## How to use this stuff
Just start terminal.py and play with it or create your own script.

Here is a very minimal draft:
```python
from pyuo import client
from pyuo import brain

class MyBrain(brain.Brain):
    ''' Override the base brain's methods to do your fancy things '''

if __name__ == '__main__':
    # Create a new client
    client = client.Client()
    # Connect to the server, get list of available servers
    servers = cclient.connect(ip_or_host, port, username, password)
    # Select first server, get list of characters
    chars = client.selectServer(servers[0]['idx'])
    # Select first character
    client.selectCharacter(chars[0]['name'], 0)
    # Finally start the main ai loop and enter it
    MyBrain(client)
