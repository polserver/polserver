/** @file
 *
 * @par History
 */


#include "../clib/esignal.h"
#include "../clib/threadhelp.h"
// #include "../clib/xmain.h"
#include "../clib/logfacility.h"

#include "../plib/systemstate.h"

#include "polresource.h"

#include "../clib/Header_Windows.h"
#include "../clib/Program/ProgramConfig.h"
#include "pol.h"
// This needs to be after the windows includes, otherwise it'll complain about windows types.
#include "../clib/NTService.h"
#include "../clib/ntservmsg.h"

#ifdef _MSC_VER
#pragma warning( disable : 4189 )  // local variable is initialized but not referenced. TODO: check
                                   // if TrackPopupMenu() returns an error on line 146.
#endif

namespace Pol
{
namespace Core
{
int RunWindowsService( int argc, char** argv );
}

int xmain( int argc, char* argv[] )
{
  return Core::RunWindowsService( argc, argv );
}
namespace Core
{
class PolService : public Clib::CNTService
{
public:
  PolService();
  virtual void Run() override;
  virtual void OnStop() override;
};

PolService::PolService() : Clib::CNTService( "POL" ) {}

void PolService::Run()
{
  // a technicality: a service generally starts with an ignorant current directory.
  try
  {
    //        threadhelp::thread_sleep_ms( 20000 );
    DWORD rc;
    char buffer[1000] = "";
    rc = GetCurrentDirectory( sizeof buffer, buffer );
    LogEvent( EVENTLOG_INFORMATION_TYPE, EVMSG_DEBUG, buffer );
    LogEvent( EVENTLOG_INFORMATION_TYPE, EVMSG_DEBUG, PROG_CONFIG::programDir().c_str() );
    rc = SetCurrentDirectory( PROG_CONFIG::programDir().c_str() );

    xmain_outer( false /*testing*/ );
  }
  catch ( std::exception& ex )
  {
    this->LogEvent( EVENTLOG_ERROR_TYPE, EVMSG_DEBUG, ex.what() );
  }
}
void PolService::OnStop()
{
  Clib::signal_exit();
}


int RunWindowsService( int argc, char** argv )
{
  // Create the service object
  PolService MyService;

  // Parse for standard arguments (install, uninstall, version etc.)
  if ( !MyService.ParseStandardArgs( argc, argv ) )
  {
    // no service-related parameters
    return xmain_outer( false /*testing*/ );
  }
  else
  {
    return 0;
  }
}


#define WM_ICON_NOTIFY WM_USER + 1
unsigned WM_TASKBARCREATED;
HWND hwnd;
NOTIFYICONDATA ndata;

#define DLG_MODELESS 1

#ifdef _M_X64
INT_PTR CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
#else
BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
#endif
{
  if ( uMsg == WM_TASKBARCREATED )
    uMsg = WM_INITDIALOG;

  switch ( uMsg )
  {
  // case WM_TASKBARCREATED:
  case WM_INITDIALOG:
    ndata.hWnd = hwndDlg;

    Shell_NotifyIcon( NIM_ADD, &ndata );

    SetWindowPos( hwndDlg, nullptr, -10, -10, 0, 0, SWP_NOZORDER | SWP_NOMOVE );
    hwnd = hwndDlg;
    break;

  case WM_ICON_NOTIFY:

    if ( lParam == WM_RBUTTONDOWN )
    {
      // Beep(200,200); // Removed beep... ! (Debugging POLLaunch made me hate this :/)
      HMENU m = LoadMenu( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDR_POPUP ) );
      HMENU subm = GetSubMenu( m, 0 );
      POINT p;
      GetCursorPos( &p );

      SetForegroundWindow( hwndDlg );
      BOOL rc = TrackPopupMenu( subm, TPM_LEFTALIGN, p.x, p.y, 0, hwndDlg, nullptr );
      PostMessage( hwndDlg, WM_NULL, 0, 0 );
    }
    break;
  case WM_COMMAND:
    if ( LOWORD( wParam ) == ID_POPUP_STOP )
      Clib::signal_exit();
    else if ( LOWORD( wParam ) == ID_POPUP_SHOWCONSOLE )
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
      nullptr,
      nullptr,
      TRUE,  // inherit handles
      CREATE_NEW_CONSOLE,
      nullptr,
      nullptr,
      &si,
      &pi );
      DWORD err = GetLastError();
      Sleep( 5000 );
      rc = WriteFile( si.hStdOutput, "abcdef", 6, nullptr, nullptr );
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
    else if ( LOWORD( wParam ) == ID_UPDATE_NOTIFYDATA )
    {
      Shell_NotifyIcon( NIM_MODIFY, &ndata );
    }
    else if ( LOWORD( wParam ) == ID_SHUTDOWN )
    {
      Shell_NotifyIcon( NIM_DELETE, &ndata );
#if DLG_MODELESS
      DestroyWindow( hwndDlg );
#else
      EndDialog( hwndDlg, wParam );
#endif
    }
    break;
  case WM_DESTROY:
  case WM_CLOSE:
  case WM_QUIT:
  {
    Shell_NotifyIcon( NIM_DELETE, &ndata );
#if DLG_MODELESS
    DestroyWindow( hwndDlg );
#else
    EndDialog( hwndDlg, wParam );
#endif
    break;
  }
  }
  return FALSE;
}

void SystemTrayDialogThread( void* )
// DWORD WINAPI Thread(  LPVOID lpParameter)
// void SystemTrayDialogThread( void )
{
// modeless:
#if DLG_MODELESS
  hwnd = CreateDialog( nullptr, MAKEINTRESOURCE( IDD_DIALOG1 ), nullptr, DialogProc );
  ShowWindow( hwnd, SW_HIDE );

  BOOL bRet;
  MSG msg;

  while ( ( bRet = GetMessage( &msg, nullptr, 0, 0 ) ) != 0 )
  {
    if ( bRet == -1 )
    {
      // handle the error and possibly exit
      break;
    }
    else if ( !IsWindow( hwnd ) || !IsDialogMessage( hwnd, &msg ) )
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
  }
#else
  // modal:
  DialogBox( nullptr, MAKEINTRESOURCE( IDD_DIALOG1 ), nullptr, DialogProc );
#endif
}

void SetSysTrayPopupText( const char* text )
{
  strncpy( ndata.szTip, text, sizeof( ndata.szTip ) - 1 );
  ndata.szTip[sizeof( ndata.szTip ) - 1] = '\0';

  if ( hwnd )
    SendMessage( hwnd, WM_COMMAND, ID_UPDATE_NOTIFYDATA, 0 );
}

BOOL WINAPI control_handler_SystemTray( DWORD dwCtrlType )
{
  switch ( dwCtrlType )
  {
  case CTRL_CLOSE_EVENT:  // console window closing
    Shell_NotifyIcon( NIM_DELETE, &ndata );
#if DLG_MODELESS
    DestroyWindow( hwnd );
#else
    EndDialog( hwndDlg, wParam );
#endif
    break;
  }
  return FALSE;
}
void InitializeSystemTrayHandling()
{
  WM_TASKBARCREATED = RegisterWindowMessage( "TaskbarCreated" );

  ndata.cbSize = sizeof( NOTIFYICONDATA );
  //  ndata.hWnd=hwndDlg;
  ndata.uID = 2000;
  ndata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  ndata.uCallbackMessage = WM_ICON_NOTIFY;
  ndata.hIcon = ::LoadIcon( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDI_POLTRAY ) );
  strcpy( ndata.szTip, "POL Initializing" );

  _beginthread( SystemTrayDialogThread, 0, nullptr );
  SetConsoleCtrlHandler( control_handler_SystemTray, TRUE );
  //    _beginthreadex( nullptr,
  //                    0,
  //                    SystemTrayDialogThread,
  //                    0,
  //                    nullptr );
  // CreateThread(nullptr,0,Thread,nullptr,0,nullptr);
  //    threadhelp::start_thread( SystemTrayDialogThread, "SystemTrayDialog" );
}

void ShutdownSystemTrayHandling()
{
  if ( hwnd )
  {
#ifdef _M_X64
    PDWORD_PTR res = nullptr;
    LRESULT rc = SendMessageTimeout( hwnd, WM_COMMAND, ID_SHUTDOWN, 0, SMTO_NORMAL, 5000, res );
#else
    DWORD res;
    LRESULT rc = SendMessageTimeout( hwnd, WM_COMMAND, ID_SHUTDOWN, 0, SMTO_NORMAL, 5000, &res );
#endif

    if ( !rc )
    {
      DWORD err = GetLastError();
      INFO_PRINTLN( "Unable to shutdown systray: {}", err );
    }
  }
}
}  // namespace Core
}  // namespace Pol
