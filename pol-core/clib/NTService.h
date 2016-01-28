/** @file
 *
 * @par History
 *
 * @note Definitions for CNTService
 */


#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_

#include "ntservmsg.h" // Event message ids

#define SERVICE_CONTROL_USER 128
namespace Pol {
  namespace Clib {
	class CNTService
	{
	public:
	  CNTService( const char* szServiceName );
	  virtual ~CNTService();
	  BOOL ParseStandardArgs( int argc, char* argv[] );
	  BOOL IsInstalled();
	  BOOL Install();
	  BOOL Uninstall();
	  void LogEvent( WORD wType, DWORD dwID,
					 const char* pszS1 = NULL,
					 const char* pszS2 = NULL,
					 const char* pszS3 = NULL );
	  BOOL StartService();
	  void SetStatus( DWORD dwState );
	  BOOL Initialize();
	  virtual void Run();
	  virtual BOOL OnInit();
	  virtual void OnStop();
	  virtual void OnInterrogate();
	  virtual void OnPause();
	  virtual void OnContinue();
	  virtual void OnShutdown();
	  virtual BOOL OnUserControl( DWORD dwOpcode );
	  void DebugMsg( const char* pszFormat, ... );

	  // static member functions
	  static void WINAPI ServiceMain( DWORD dwArgc, LPTSTR* lpszArgv );
	  static void WINAPI Handler( DWORD dwOpcode );

	  // data members
	  char m_szServiceName[64];
	  int m_iMajorVersion;
	  int m_iMinorVersion;
	  SERVICE_STATUS_HANDLE m_hServiceStatus;
	  SERVICE_STATUS m_Status;
	  BOOL m_bIsRunning;

	  // static data
	  static CNTService* m_pThis; // nasty hack to get object ptr

	private:
	  HANDLE m_hEventSource;

	};
  }
}
#endif // _NTSERVICE_H_
