/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include <windows.h>
#include <process.h>

#include "clib/esignal.h"
#include "clib/ntservice.h"
#include "clib/progver.h"
#include "clib/threadhelp.h"
#include "clib/xmain.h"

#include "plib/polver.h"

//#include "polver.h"

class PolService : public CNTService
{
public:
	PolService();
    virtual void Run();
    virtual void OnStop();
};

PolService::PolService()
:CNTService("POL")
{
}

extern char* whereami;
int xmain_outer( int argc, char* argv[] );

void PolService::Run()
{
    // a technicality: a service generally starts with an ignorant current directory.
    try
    {
//        threadhelp::thread_sleep_ms( 20000 );
        DWORD rc;
        char buffer[ 1000 ] = "";
        rc = GetCurrentDirectory( sizeof buffer, buffer );
        LogEvent( EVENTLOG_INFORMATION_TYPE, EVMSG_DEBUG, buffer );
        LogEvent( EVENTLOG_INFORMATION_TYPE, EVMSG_DEBUG, xmain_exedir.c_str() );
        rc = SetCurrentDirectory( xmain_exedir.c_str() );

        char* dummy[1] = { "" };
        xmain_outer(1, dummy);
    }
    catch( std::exception& ex )
    {
        this->LogEvent( EVENTLOG_ERROR_TYPE, EVMSG_DEBUG, ex.what() ); 
    }
}
void PolService::OnStop()
{
    exit_signalled = true;
}


int RunWindowsService( int argc, char** argv )
{
    // Create the service object
    PolService MyService;
    
    // Parse for standard arguments (install, uninstall, version etc.)
    if (!MyService.ParseStandardArgs(argc, argv))
    {
        // no service-related parameters
        return xmain_outer( argc, argv );
    }
    else
    {
        return 0;
    }
}

int xmain( int argc, char *argv[] )
{
    strcpy( progverstr, polverstr );
    strcpy( buildtagstr, polbuildtag );
    progver = polver;

    return RunWindowsService( argc, argv );
}

#include "polresource.h"

#define WM_ICON_NOTIFY	WM_USER+1
unsigned WM_TASKBARCREATED;
HWND					hwnd;
NOTIFYICONDATA ndata;

#define DLG_MODELESS 1

BOOL CALLBACK DialogProc(  HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_TASKBARCREATED)
        uMsg = WM_INITDIALOG;

    switch (uMsg)
	{
    //case WM_TASKBARCREATED:
	case WM_INITDIALOG:
    	ndata.hWnd=hwndDlg;
		
		Shell_NotifyIcon(NIM_ADD,&ndata);
		
		SetWindowPos(hwndDlg,NULL,-10,-10,0,0,SWP_NOZORDER|SWP_NOMOVE);
		hwnd=hwndDlg;
		break;
	
	case WM_ICON_NOTIFY:
		
		if (lParam==WM_RBUTTONDOWN)
		{
			//Beep(200,200); // Removed beep... ! (Debugging POLLaunch made me hate this :/)
            HMENU m = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE( IDR_POPUP ) );
            HMENU subm = GetSubMenu( m, 0 );
            POINT p;
            GetCursorPos( &p );
            RECT r;
            r.bottom = r.top = r.left = r.right = 0;
            SetForegroundWindow( hwndDlg );
            BOOL rc = TrackPopupMenu( subm, TPM_LEFTALIGN, p.x, p.y, 0, hwndDlg, &r );
            PostMessage(hwndDlg, WM_NULL, 0, 0); 
			/*
            CMenu menu;
			menu.LoadMenu(IDR_POPUP);
			CMenu* popup=menu.GetSubMenu(0);
			CPoint pt;
			GetCursorPos(&pt);
			popup->TrackPopupMenu(TPM_LEFTALIGN,pt.x,pt.y,CWnd::FromHandle(hwndDlg),CRect(0,0,0,0));
            */
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_POPUP_STOP)
			exit_signalled = true;
        else if (LOWORD(wParam) == ID_POPUP_SHOWCONSOLE)
        {
/*
PROCESS_INFORMATION pi;
            STARTUPINFO si;
            
            ZeroMemory(&si,sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
            BOOL rc;
            rc = CreateProcess( "constub.exe", 
                           "constub.exe", 
                           NULL, 
                           NULL, 
                           TRUE,  // inherit handles
                           CREATE_NEW_CONSOLE, 
                           NULL,
                           NULL, 
                           &si, 
                           &pi );
            DWORD err = GetLastError();
            Sleep( 5000 );
            rc = WriteFile( si.hStdOutput, "abcdef", 6, NULL, NULL );
*/
/*
HWND x = GetConsoleWindow();
            HANDLE hInput = GetStdHandle( STD_INPUT_HANDLE );
            HANDLE hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
            HANDLE hError = GetStdHandle( STD_ERROR_HANDLE );

            //FreeConsole();
            AllocConsole();
            cout << "cout xyz" << endl;
            cerr << "cerr xyz" << endl;
            printf( "Hi there.\n" );
            fflush( stdout );
*/
        }
        else if (LOWORD(wParam) == ID_UPDATE_NOTIFYDATA)
        {
    		Shell_NotifyIcon(NIM_MODIFY,&ndata);
        }
        else if (LOWORD(wParam) == ID_SHUTDOWN)
        {
            Shell_NotifyIcon(NIM_DELETE, &ndata );
#if DLG_MODELESS
            DestroyWindow(hwndDlg); 
#else
            EndDialog( hwndDlg, wParam );
#endif
        }
		break;
	}
	return FALSE;
}

void SystemTrayDialogThread( void* )
//DWORD WINAPI Thread(  LPVOID lpParameter)
//void SystemTrayDialogThread( void )
{
// modeless:
#if DLG_MODELESS
   	HWND hwnd=CreateDialog(NULL,MAKEINTRESOURCE(IDD_DIALOG1),NULL,DialogProc);
    ShowWindow( hwnd, SW_HIDE );
    
    BOOL bRet;
    MSG msg;

    while ( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0 ) 
    { 
        if (bRet == -1 )
        {
            // handle the error and possibly exit
            break;
        }
        else if (!IsWindow(hwnd) || !IsDialogMessage(hwnd, &msg)) 
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        } 
    } 
#else
// modal:
    DialogBox(NULL,MAKEINTRESOURCE(IDD_DIALOG1),NULL,DialogProc);
#endif
}

void SetSysTrayPopupText( const char* text )
{
    strncpy( ndata.szTip, text, sizeof ndata.szTip );
    if (hwnd)
        SendMessage( hwnd, WM_COMMAND, ID_UPDATE_NOTIFYDATA, 0 );
}

void InitializeSystemTrayHandling()
{
    WM_TASKBARCREATED = RegisterWindowMessage( "TaskbarCreated" );

    ndata.cbSize=sizeof(NOTIFYICONDATA);
//	ndata.hWnd=hwndDlg;
	ndata.uID=2000;
	ndata.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	ndata.uCallbackMessage=WM_ICON_NOTIFY;
	ndata.hIcon=::LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_POLTRAY));
	strcpy(ndata.szTip,"POL Initializing");			

    _beginthread( SystemTrayDialogThread, 0, NULL );
//    _beginthreadex( NULL, 
//                    0, 
//                    SystemTrayDialogThread, 
//                    0, 
//                    NULL );
    // CreateThread(NULL,0,Thread,NULL,0,NULL);
//    threadhelp::start_thread( SystemTrayDialogThread, "SystemTrayDialog" );
}

void ShutdownSystemTrayHandling()
{
    if (hwnd)
    {
        DWORD res;
        LRESULT rc = SendMessageTimeout( hwnd, WM_COMMAND, ID_SHUTDOWN, 0, SMTO_NORMAL, 5000, &res );
        if (!rc)
        {
            DWORD err = GetLastError();
            cout << "Unable to shutdown systray: " << err << endl;
        }
    }
}
