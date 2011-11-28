/******************************************************************************\
* 
* 
*  Copyright (C) 2004 Daniel 'Necr0Potenc3' Cavalcanti
* 
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
* 
* 	Dec 28th, 2004 -- UOCH was released 2 years ago, coded in VB. This is the
*   improved version, written in C
*   Jan 16th, 2005 -- added loginkeys searching to Developer's Dump
*   Jan 21th, 2005 -- Changes to CrackTools, added \" \" to all string refs (%s)
*   all vars that hold returns from CrackTools are now initialized as -1
*   changed the "ioh" declaration to avoid warnings/errors
*   wrote the function WriteFileSequence, its cleaner this way
* 
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "Main.h"
#include "CrackTools.h"
#include "Crc.h"


HINSTANCE CurInstance = NULL;
HWND CurWnd = NULL;

unsigned char *ClientBuf = NULL;
unsigned int ClientSize = 0;
unsigned int ImageBase = 0;
unsigned int EntryPoint = 0;
unsigned int TimeDateStamp = 0;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	CurInstance = hInstance;

	DialogBox(CurInstance, MAKEINTRESOURCE(DIALOG_MAIN), 0, (DLGPROC)MainDlgProc);

	return 1;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	CurWnd = hDlg;

	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			char ClientPath[4096];
			HKEY Key = NULL;
			BOOL RegOK = TRUE;
			DWORD Size = sizeof(ClientPath);

			/* set the UOCH icon :) */
			SendMessage(CurWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(CurInstance, MAKEINTRESOURCE(NECRO_ICON)));
			
			SendMessage(GetDlgItem(CurWnd, CHECK_CRYPT), BM_SETCHECK, BST_CHECKED, 0);

			/* see if we can open uo's registry key */
			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Origin Worlds Online\\Ultima Online\\1.0", 0, KEY_READ, &Key) != ERROR_SUCCESS)
				if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0", 0, KEY_READ, &Key) != ERROR_SUCCESS)
					RegOK = FALSE;

			/* if we can, get the client path and set it to the text box */
			if(RegOK)
			{
				RegQueryValueEx(Key, "ExePath", NULL, NULL/*type*/, (LPBYTE)ClientPath, &Size);
				RegCloseKey(Key);

				SetDlgItemText(CurWnd, EDIT_PATH, ClientPath);
			}

			return TRUE;
		}break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case BUTTON_PATH:
				{
					OPENFILENAME ofn;
					char ClientPath[4096];

					/* ask for the client's path */
					memset(ClientPath, 0, sizeof(ClientPath));
					memset(&ofn, 0, sizeof(ofn));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = CurWnd;
					ofn.lpstrFile = ClientPath;
					ofn.nMaxFile = sizeof(ClientPath);
					ofn.lpstrFilter = "Exe\0*.Exe\0All\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrTitle = "Choose the UO client to be patched";
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					GetOpenFileName(&ofn);

					/* set it */
					SetDlgItemText(CurWnd, EDIT_PATH, ClientPath);

					return TRUE;
				}break;
				case BUTTON_PATCH:
				{
					IMAGE_DOS_HEADER *idh = NULL;
					IMAGE_FILE_HEADER *ifh = NULL;
					IMAGE_OPTIONAL_HEADER *ioh = NULL;

					HANDLE ClientHandle = NULL;
					DWORD BytesRead = 0;
					char ClientPath[4096], NewClientPath[4096];
					char *Tmp = NULL;
					int i = 0;

					DWORD TickCount = 0;

					memset(NewClientPath, 0, sizeof(NewClientPath));
					GetDlgItemText(CurWnd, EDIT_PATH, ClientPath, sizeof(ClientPath));

					/* copy the client to a buffer */
					ClientHandle = CreateFile(ClientPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
					if(ClientHandle == INVALID_HANDLE_VALUE)
					{
						MBOut("Yet another uneducated soul uses me...", "Could not open the file \"%s\". Check if the path is right and the file is not being used", ClientPath);
						EndDialog(CurWnd, 0);
						return TRUE;
					}

					ClientSize = GetFileSize(ClientHandle, NULL);
					ClientBuf = (unsigned char *)malloc(ClientSize);
					if(ClientBuf == NULL)
					{
						MBOut("Shice", "Couldn't allocate enough memory (%d bytes) for the client's image (handle: %X).. weird.", ClientSize, ClientHandle);
						CloseHandle(ClientHandle);
						PostQuitMessage(0);
						return TRUE;
					}

					ReadFile(ClientHandle, ClientBuf, ClientSize, &BytesRead, NULL);
					CloseHandle(ClientHandle);

					/*
					* find the imagebase (virtual address of the module in memory when loaded)
					* and the entrypoint
					*/
					idh = (IMAGE_DOS_HEADER *)ClientBuf;
					ifh = (IMAGE_FILE_HEADER *)(ClientBuf + idh->e_lfanew + sizeof(IMAGE_NT_SIGNATURE));
					ioh = (IMAGE_OPTIONAL_HEADER *)((ClientBuf + idh->e_lfanew + sizeof(IMAGE_NT_SIGNATURE)) + sizeof(IMAGE_FILE_HEADER));

					if((*(DWORD*)(ClientBuf + idh->e_lfanew)) != IMAGE_NT_SIGNATURE)
					{
						MBOut("Ti Durak", "The client is not a valid PE");
						PostQuitMessage(0);
						return TRUE;
					}

					TimeDateStamp = ifh->TimeDateStamp;
					ImageBase = ioh->ImageBase;
					EntryPoint = ioh->AddressOfEntryPoint;

					TickCount = GetTickCount();

					TextBoxCat(CurWnd, TEXT_DUMP, "Patching...\r\n");
					Sleep(25);

					/* now check which patches need to be done and execute them */
					if(SendMessage(GetDlgItem(CurWnd, CHECK_CRYPT), BM_GETCHECK, 0, 0) == BST_CHECKED)
						RemoveEncryption();
					if(SendMessage(GetDlgItem(CurWnd, CHECK_STAMINA), BM_GETCHECK, 0, 0) == BST_CHECKED)
						RemoveStaminaCheck();
					if(SendMessage(GetDlgItem(CurWnd, CHECK_WEATHER), BM_GETCHECK, 0, 0) == BST_CHECKED)
						RemoveWeatherMessages();

					/* create a new client */
					/* remove the exe name from the path */
					if((Tmp = strrchr(ClientPath, '\\')) != NULL)
						*Tmp = '\0';

					sprintf(NewClientPath, "%s\\UOCH_n0p3_Client", ClientPath);
					Tmp = WriteFileSequence(NewClientPath, ClientBuf, ClientSize);

					if(Tmp == NULL)
						TextBoxCat(CurWnd, TEXT_DUMP, "Could not write: \"%s\" GetLastError: %d\r\n", NewClientPath, GetLastError);
					else
					{
						TextBoxCat(CurWnd, TEXT_DUMP, "New UOCH client: \"%s\"\r\n", Tmp);
						TextBoxCat(CurWnd, TEXT_DUMP, "Time elapsed during patching: %dms\r\n\r\n", GetTickCount() - TickCount);
					}

					free(ClientBuf);
					ClientBuf = NULL;

					return TRUE;
				}break;
			}
		}break;
		case WM_DROPFILES:
		{
			char Path[4096];

			/* a text box will only pass the WM_DROPFILES to the owner if it's set as multiline and no accept files flag */
			DragQueryFile((HDROP)wParam, 0, Path, sizeof(Path));
			DragFinish((HDROP)wParam);

			SetDlgItemText(CurWnd, EDIT_PATH, Path);
			return TRUE;
		}break;
		case WM_NOTIFY:
			{
				MBOut("", "noitify");
			}break;
		case WM_CLOSE:
		{
			EndDialog(CurWnd, 0);
			return TRUE;
		}break;
	}

	return FALSE;
}


void TextBoxCat(HWND Dlg, int DlgItem, const char *Text, ...)
{
	char Final[4096];
	va_list List;

	char *NewText = NULL;
	unsigned int NewTextSize = 0;

	va_start(List, Text);
	vsprintf((char*)Final, Text, List);
	va_end(List);

	/* get the dlg text, make sure we have enough space for it */
	NewTextSize += 100;
	NewText = (char*)realloc(NewText, NewTextSize);
	while(GetDlgItemText(Dlg, DlgItem, NewText, NewTextSize) == (int)NewTextSize - 1)
	{
		NewTextSize += 100;
		NewText = (char*)realloc(NewText, NewTextSize);
	}

	NewTextSize += (int)(strlen(Final) + 1);
	NewText = (char*)realloc(NewText, NewTextSize);
	strcat(NewText, Final);
	SetDlgItemText(Dlg, DlgItem, NewText);

	free(NewText);
	return;
}

void MBOut(char *title, char *msg, ...)
{
	char final[4096];
	va_list list;
	va_start(list, msg);
	vsprintf((char*)final, msg, list);
	va_end(list);

	MessageBox(NULL, final, title, 0);
	
	return;
}

/* Path should not end with <.filetype>. returns the name of the new file or NULL */
char* WriteFileSequence(char *Path, char *Buf, unsigned int BufSize)
{
	HANDLE FHandle = INVALID_HANDLE_VALUE;
	DWORD BytesRead = 0;
	static char NewPath[4096];
	int i = 0;

	/* try to find an unexisting name for the new exe */
	for(i = 0; i < 0xFFFF; i++)
	{
		sprintf(NewPath, "%s%02d.exe", Path, i);

		FHandle = CreateFile(NewPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
		if(FHandle != INVALID_HANDLE_VALUE)
			break;
		else if(FHandle == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS)
			return NULL;
	}

	WriteFile(FHandle, Buf, BufSize, &BytesRead, NULL);
	CloseHandle(FHandle);

	return NewPath;
}

void RemoveEncryption(void)
{
	/* for login encryption */
	unsigned char CryptSig[8] = { 0x81, 0xF9, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x8F };
	unsigned char CryptSigNew[8] = { 0x00, 0x00, 0x00, 0x00, 0x75, 0x12, 0x8b, 0x54 };
	unsigned char JNZSig[2] = { 0x0F, 0x85 };
	unsigned char JNESig[2] = { 0x0F, 0x84 };
	unsigned int CryptPos = -1, CryptPosNew = -1, JNZPos = -1, JEPos = -1;
	unsigned int NewClient = -1;
	

	/* for game encryption */
	unsigned char BFGamecryptSig[5] = { 0x2C, 0x52, 0x00, 0x00, 0x76 }; /* CMP XXX, 522c - JBE */
	unsigned char CmpSig[4] = { 0x3B, 0xC3, 0x0F, 0x84 }; /* CMP EAX,EBX - JE */
	unsigned int BFGamecryptPos = -1, CmpPos = -1;
	/* RTD: make sure the JE 0x10 and JE 0x9X000000 stays like that, if not... I have to find a new way */
	unsigned char TFGamecryptSig[14] = { 0x8B, 0x8B, 0xCC, 0xCC, 0xCC, 0xCC, 0x81, 0xF9, 0x00, 0x01, 0x00, 0x00, 0x74, 0x10 }; /* MOV EBX, XXX - CMP ECX, 0x100 - JE 0x10 */
	unsigned char TFGamecryptSigNew[9] = { 0x74, 0x0f, 0x83, 0xb9, 0xb4, 0x00, 0x00, 0x00, 0x00 };
	unsigned char JELong[7] = { 0x0F, 0x84, 0xCC, 0x00, 0x00, 0x00, 0x55 }; /* JE XX000000 -  */
	unsigned int TFGamecryptPos = -1, TFGamecryptPosNew = -1, GJEPos = -1;

	/* for game decryption */
	unsigned char DecryptSig[8] = { 0x4A, 0x83, 0xCA, 0xF0, 0x42, 0x8A, 0x94, 0x32 };
	unsigned char DectestSig[10] = { 0x85, 0xCC, 0x74, 0xCC, 0x33, 0xCC, 0x85, 0xCC, 0x7E, 0xCC };
	unsigned char DecryptSigNew[11] = { 0x00, 0x00, 0x74, 0x37, 0x83, 0xbe, 0xb4, 0x00, 0x00, 0x00, 0x00 };
	unsigned int DecryptPos = -1, DectestPos = -1, DecryptPosNew = -1;

	/* LOGINENCRYPTION START */

	/*
	* magic x90 encryption signature: 81 f9 00 00 01 00 0f 8f
	* patching : find the first 0f 84 bellow and change it to 0f 85
	* or first 0f 85 and change it to 0f 84
	*/
	CryptPos = FleXSearch(CryptSig, ClientBuf, sizeof(CryptSig), ClientSize, 0, 0x100, 1);
	CryptPosNew = FleXSearch(CryptSigNew, ClientBuf, sizeof(CryptSigNew), ClientSize, 0, 0x100, 1);

	/* search for the first JNZ or JNE from crypt_addr */
	JNZPos = FleXSearch(JNZSig, ClientBuf, sizeof(JNZSig), ClientSize, CryptPos, 0x100, 1);
	JEPos = FleXSearch(JNESig, ClientBuf, sizeof(JNESig), ClientSize, CryptPos, 0x100, 1);

	/* if this is an OSI UO client, shouldnt happen AT ALL */
	if(CryptPos == -1 && CryptPosNew == -1)
	{
		TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: could not find the sig... weird\r\n");
    }	
	else
	{
        if(CryptPos != -1)
        {
            TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: Old crypt sig @%X\r\n", CryptPos);

            if(!JEPos || JEPos > JNZPos)
            {
			    ClientBuf[JNZPos + 1] = 0x84;
			    TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: Patched with je @%X\r\n", JNZPos);
	    	}
		    else if(!JNZPos || JNZPos > JEPos)
		    {
		    	ClientBuf[JEPos + 1] = 0x85;
		    	TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: Patched with jnz @%X\r\n", JEPos);
	    	}
         }
         else if(CryptPosNew != -1)
         {	
             TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: New crypt sig @%X\r\n", CryptPosNew);   
	    	 ClientBuf[CryptPosNew + 4] = 0xEB;
	    	 TextBoxCat(CurWnd, TEXT_DUMP, "LCrypt: Patched @%X\r\n", CryptPosNew);
	    	 NewClient = 1;
         }     
	}

	/* LOGINENCRYPTION END */

	/* GAMEENCRYPTION START */

	/*
	* BLOWFISH
	* this is simply "lost" inside the "send" function, just above it
	* it looks like:
	* if(GameMode != LOGIN_SOCKET) ;game socket
	* {
	*     BlowfishEncrypt() ;starts with a (while > 0x522c)
	*     ;a whole bunch of other stuff
	*     TwofishEncrypt() ;if present
	*     send()
	* }
	* else ;login socket
	*     send() ;yay, a send that bypasses all the encryption crap
	*
	* find the beginning of the loop while(Obj->stream_pos + len > CRYPT_GAMETABLE_TRIGGER)
	* CRYPT_GAMETABLE_TRIGGER is 0x522c
	* find the first CMP EAX,EBX-JE above and patch it to CMP EAX,EAX
	*/
	BFGamecryptPos = FleXSearch(BFGamecryptSig, ClientBuf, sizeof(BFGamecryptSig), ClientSize, 0, 0x100, 1);
	CmpPos = FleXSearch(CmpSig, ClientBuf, sizeof(CmpSig), ClientSize, BFGamecryptPos - 0x20, 0x100, 1);

	if(BFGamecryptPos == -1 || CmpPos == -1)
		TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Could not find blowfish encryption %X %X\r\n", BFGamecryptPos, CmpPos);
	else
	{
		ClientBuf[CmpPos + 1] = 0xC0;
		TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Found blowfish @%X\r\n", BFGamecryptPos);
		TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Patched with cmp @%X\r\n", CmpPos);
	}

	
	/* 
	* TWOFISH
	* patch the encryption function to skip encryption
	* the function is always called before send
	*
	* find the beginning of the loop and the first JE above it
	* patch the JE (0x84) to JNE (0x85)
	*/

	TFGamecryptPos = FleXSearch(TFGamecryptSig, ClientBuf, sizeof(TFGamecryptSig), ClientSize, 0, 0xCC, 1);
	GJEPos = FleXSearch(JELong, ClientBuf, sizeof(JEPos), ClientSize, TFGamecryptPos - 0x20, 0xCC, 1);
	TFGamecryptPosNew = FleXSearch( TFGamecryptSigNew, ClientBuf, sizeof(TFGamecryptSigNew), ClientSize, 0, 0xCC, 1);

    if(TFGamecryptPos == -1 && GJEPos == -1 && TFGamecryptPosNew == -1)
    {
		TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Could not find the Old twofish encryption %X %X\r\n", TFGamecryptPos, GJEPos);
		TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Could not find the New twofish encryption %X %X\r\n", TFGamecryptPosNew, TFGamecryptPosNew);  
    }   
    else
    {
            	if(TFGamecryptPos != -1 && GJEPos != -1)
                {
                    ClientBuf[GJEPos + 1] = 0x85;
                    TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Found Old twofish @%X\r\n", TFGamecryptPos);
                    TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Patched with jnz @%X\r\n", GJEPos);
                }    
                else if(TFGamecryptPosNew != -1)
                {
                     ClientBuf[TFGamecryptPosNew] = 0xEB;
                     TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Found New twofish @%X\r\n", TFGamecryptPosNew);
                     TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Patched @%X\r\n", TFGamecryptPosNew);   
                }   
    }         	  
	

	/* GAMEENCRYPTION END */

	/* GAMEDECRYPTION START (now for the easy part) */

	/*
	* search for 4A 83 CA F0 42 8A 94 32
	* and above it, 85 xx 74 xx 33 xx 85 xx 7E xx
	* the first TEST (85 xx) must be cracked to CMP EAX, EAX (3B C0)
	* if I want to do it like LB does in UORice, I'd crack
	* the first CMP xx JMP xx (85 xx 74 xx) to CMP EAX, EAX (3B C0)
	* which is bellow the one I crack
	*/

	/* find 4A 83 CA F0 42 8A 94 32 */
	/* find the TEST above it (not the one right above that is) */
	DecryptPos = FleXSearch(DecryptSig, ClientBuf, sizeof(DecryptSig), ClientSize, 0, 0x100, 1);
	DecryptPosNew = FleXSearch(DecryptSigNew, ClientBuf, sizeof(DecryptSigNew), ClientSize, 0, 0x100, 1);
	DectestPos = FleXSearch(DectestSig, ClientBuf, sizeof(DectestSig), ClientSize, DecryptPos - 0x100, 0xCC, 1);

	if(DecryptPos == -1 && DectestPos == -1 && DecryptPosNew == -1)
		TextBoxCat(CurWnd, TEXT_DUMP, "GDecrypt: Could not find MD5 decryption %X %X \r\n", DecryptPos, DecryptPos, DectestPos);
	else
	{
        if(NewClient == -1)
        {
		    DectestPos = FleXSearch(DectestSig, ClientBuf, sizeof(DectestSig), ClientSize, DecryptPos - 0x100, 0xCC, 1);
            ClientBuf[DectestPos] = 0x3B;
            TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Found Old MD5 @%X\r\n", DecryptPos);
            TextBoxCat(CurWnd, TEXT_DUMP, "GDecrypt: Sig @%X patched with cmp @%X\r\n", DecryptPos, DectestPos);
        }
        else if(NewClient == 1)
        {
            ClientBuf[DecryptPosNew + 2] = 0xEB;
            TextBoxCat(CurWnd, TEXT_DUMP, "GCrypt: Found New MD5 @%X\r\n", DecryptPos);
            TextBoxCat(CurWnd, TEXT_DUMP, "GDecrypt: Sig @%X patched @%X\r\n", DecryptPos, DectestPos);
        }         
	}

	/* GAMEDECRYPTION END */

	return;
}

void RemoveStaminaCheck(void)
{
	/*
	* client 4.0.4b
	* 004EA7A6  |. 85C0           |TEST EAX,EAX
	* 004EA7A8  |. 74 09          |JE SHORT client40.004EA7B3
	* 004EA7AA  |. 7E 36          |JLE SHORT client40.004EA7E2
	* 004EA7AC  |. 83F8 02        |CMP EAX,2
	* 004EA7AF  |. 7F 31          |JG SHORT client40.004EA7E2
	* 004EA7B1  |. EB 5D          |JMP SHORT client40.004EA810
	* 004EA7B3  |> 8B91 FC010000  |MOV EDX,DWORD PTR DS:[ECX+1FC]
	* 004EA7B9  |. 8B81 00020000  |MOV EAX,DWORD PTR DS:[ECX+200]
	* 004EA7BF  |. 3BD0           |CMP EDX,EAX
	* 004EA7C1  |. 74 4D          |JE SHORT client40.004EA810
	*
	* if I remember it right, at the end EDX has the current stamina, EAX the full
	* it checks if they are equal and if they are it allows you to walk
	* through other players
	* patch the CMP EDX,EAX (3B D0) to CMP EAX,EAX (3B C0)
	* hooligan mode :P
	* thanks to Bloodbob (bloob) for this one
	*/

	unsigned char StaminacheckSigOld[25] = { 0x74, 0xCC, 0x7E, 0xCC, 0x83, 0xF8, 0x02, 0x7F, 0xCC, 0xEB, 0xCC, 0x8B, 0x91, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x81, 0xCC, 0xCC, 0xCC, 0xCC, 0x3B, 0xD0 };
	unsigned char StaminacheckSigNew[25] = { 0x74, 0xCC, 0x7E, 0xCC, 0x83, 0xF8, 0x02, 0x7F, 0xCC, 0xEB, 0xCC, 0x8B, 0x91, 0xCC, 0xCC, 0xCC, 0xCC, 0x3B, 0x91, 0xCC, 0xCC, 0xCC, 0xCC, 0x74, 0xCC };
	unsigned int StaminacheckPosOld = -1, StaminacheckPosNew = -1;

	StaminacheckPosOld = FleXSearch(StaminacheckSigOld, ClientBuf, sizeof(StaminacheckSigOld), ClientSize, 0, 0xCC, 1);
	StaminacheckPosNew = FleXSearch(StaminacheckSigNew, ClientBuf, sizeof(StaminacheckSigNew), ClientSize, 0, 0xCC, 1);

    if(StaminacheckPosOld == -1 && StaminacheckPosNew == -1)
	{
		TextBoxCat(CurWnd, TEXT_DUMP, "Could not find the Old Stamina Check %X %X\r\n", StaminacheckPosOld, StaminacheckPosOld);
		TextBoxCat(CurWnd, TEXT_DUMP, "Could not find the New Stamina Check %X %X\r\n", StaminacheckPosNew, StaminacheckPosNew);
		return;
	}
	else if(StaminacheckPosOld != -1)
	{
    	ClientBuf[StaminacheckPosOld + 24] = 0xC0;
    	TextBoxCat(CurWnd, TEXT_DUMP, "Stamina: Found Old StaminaCheck @%X\r\n", StaminacheckPosOld);
     	TextBoxCat(CurWnd, TEXT_DUMP, "Stamina Check patched  @%X\r\n", StaminacheckPosOld);
    }    	
    else if(StaminacheckPosNew != -1)
    {
         ClientBuf[StaminacheckPosNew + 23] = 0xEB;
         TextBoxCat(CurWnd, TEXT_DUMP, "Stamina: Found New StaminaCheck @%X\r\n", StaminacheckPosNew);
         TextBoxCat(CurWnd, TEXT_DUMP, "Stamina Check patched @%X\r\n", StaminacheckPosNew);
    }     

	return;
}

void RemoveWeatherMessages(void)
{
     /* Removes the messges sent by Weather Packet 0x65 */
     
     unsigned int Weather_1_Pos = -1, Weather_2_Pos = 2, Weather_3_Pos = -1, Weather_4_Pos = -1;
     
     Weather_1_Pos = FleXSearch("A fierce storm approaches.", ClientBuf, (DWORD)strlen("A fierce storm approaches."), ClientSize, 0, 0xCC, 1);
     Weather_2_Pos = FleXSearch("A storm is brewing.", ClientBuf, (DWORD)strlen("A storm is brewing."), ClientSize, 0, 0xCC, 1);
     Weather_3_Pos = FleXSearch("It begins to snow.", ClientBuf, (DWORD)strlen("It begins to snow."), ClientSize, 0, 0xCC, 1);
     Weather_4_Pos = FleXSearch("It begins to rain.", ClientBuf, (DWORD)strlen("It begins to rain."), ClientSize, 0, 0xCC, 1);
     
     if(Weather_1_Pos == -1)
         TextBoxCat(CurWnd, TEXT_DUMP, "Couldn't find the 'A fierce storm approaches' string. %s %s\r\n", Weather_1_Pos, Weather_1_Pos);
     else
         strcpy(ClientBuf + Weather_1_Pos, "                          ");
         TextBoxCat(CurWnd, TEXT_DUMP, "Found 'A fierce storm is brewing' string and removed it. \r\n");
     
     if(Weather_2_Pos == -1)
         TextBoxCat(CurWnd, TEXT_DUMP, "Couldn't find the 'A storm is brewing' string. %s %s\r\n", Weather_2_Pos, Weather_2_Pos);
     else
         strcpy(ClientBuf + Weather_2_Pos, "                   ");
         TextBoxCat(CurWnd, TEXT_DUMP, "Found 'A storm is brewing' string and removed it. \r\n");
         
     if(Weather_3_Pos == -1)
         TextBoxCat(CurWnd, TEXT_DUMP, "Couldn't find the 'It begins to snow' string. %s %s\r\n", Weather_3_Pos, Weather_3_Pos);
     else
         strcpy(ClientBuf + Weather_3_Pos, "                  ");
         TextBoxCat(CurWnd, TEXT_DUMP, "Found 'It begins to snow' string and removed it. \r\n");
         
     if(Weather_4_Pos == -1)
         TextBoxCat(CurWnd, TEXT_DUMP, "Couldn't find the 'It begins to rain' string. %s %s\r\n", Weather_4_Pos, Weather_4_Pos);
     else
         strcpy(ClientBuf + Weather_4_Pos, "                  ");
         TextBoxCat(CurWnd, TEXT_DUMP, "Found 'It begins to rain' string and removed it. \r\n");            
}     
