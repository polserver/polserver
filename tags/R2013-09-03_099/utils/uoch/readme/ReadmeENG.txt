January 1st, 2005


Ultima Online Client Hacker (UOCH) 2.1
Developer's edition release

Copyright 2005 Daniel 'Necr0Potenc3' Cavalcanti


[How to use]
1 - run UOCH.exe
2 - it autodetects UO's client in the registry. but you can select another client
with the "..." button
3 - select the patches you want (Developer dump overrides cancels all patches)
4 - hit the "patch" button
5 - a new client named "UOCH_n0p3_Clientxxx.exe" will be created in the folder
of the client you patched. the xxx is the id number, you can create&patch as
many clients you want

[Features]
- Open source

- Remove encryption: Removes the client's encryption and informs you witch
encryption is being removed (Login, Blowfish, Twofish, MD5).

- Remove stamina check: Let's you walk through people even though your stamina
isn't full.!


[Source code]
   It's distributed with the executable and you are free to change it anyway
you like Just make sure you give me credits, k?

   The source code contains CrackTools.c. A collection of functions, which I
developed, useful for cracking. 
PS: All of them return the offsets inside the buffer, not the virtual address
(image base + offset)