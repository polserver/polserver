Create_Debug_Context();

    // get a process scripting object by PID
    // If no PID is supplied will use parent script pid.
GetProcess( pid := -1 );

    // getpid: get this script's pid
GetPid();

    //
    // unload_scripts: unload scripts from the script cache (they will be
    //                 reloaded from disk on demand) currently running
    //                 scripts will continue as normal.
    //                 Passing "" will unload all scripts.
    //
Unload_Scripts( scriptname := "" );

    //
    // set_script_option: Set script options
    //
Set_Script_Option( optnum, optval );	// return value is the value that is currently set for that option, or error
const SCRIPTOPT_NO_INTERRUPT := 1;      // if 1, script runs until it sleeps
const SCRIPTOPT_DEBUG        := 2;      // if 1, prints any debug info included
const SCRIPTOPT_NO_RUNAWAY   := 3;      // if 1, doesn't warn about runaway conditions
const SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES := 4;
const SCRIPTOPT_AUXSVC_ASSUME_STRING := 5;
const SCRIPTOPT_SURVIVE_ATTACHED_DISCONNECT := 6; // if 1, do not kill script if attached character's client disconnects

    //
    // set_script_option(SCRIPTOPT_NO_INTERRUPT,1) is the same as set_critical(1)
    // set_script_option(SCRIPTOPT_DEBUG,1) is the same as set_debug(1)
    //

Sleep( num_seconds );
Sleepms( num_milliseconds );

    //
    // wait_for_event: sleep for a number of seconds until an event shows up
    //                 if timeout is 0, return immediately
    //                 returns 0 if no event was ready
    //
Wait_For_Event( num_seconds_timeout );

    //
    // events_waiting: the number of events waiting, 0+
    //
Events_Waiting();

    //
    // set_priority: the priority of a script is how many instructions it
    //               executes before switching to another script.
    //               default script priority is 1.
    //               priority range is 1 to 255.
    //               Returns previous priority.
Set_Priority( priority );

    //
    // set_critical: critical scripts run if they are not blocked, without
    //               interruption.  An infinite loop in a critical script
    //               will hang the server
    //
Set_Critical( critical );


    //
    // set_debug(debug): if debug=1, and the script was compiled with
    //                   'ecompile -i [script].src', each script source line
    //                   will be printed as it is executed.
    //                   if debug=0, disables this output.
    //
Set_Debug( debug );


Start_Script( script_name, param := 0 );
Start_Skill_Script( chr, attr_name, script_name := "", param := 0 );
Run_Script_To_Completion( script_name, param := 0 );
Run_Script( script_name, param := 0 );


    //
    // syslog(text): write text to the console, and to the log file
    //               includes context (calling script name)
    //
SysLog( text, log_verbose:=1 );

	//
	// clear_event_queue(): Empties the event queue of the current script.
	//
Clear_Event_Queue();

	//
	// set_event_queue_size(size): Sets the event queue size of the current script (default 20)
	//
Set_Event_Queue_Size(size);

    //
    // is_critical(): returns 1 if the calling script is set critical, else 0.
    //
Is_Critical();

OpenURL( character, url );
OpenConnection( host, port, scriptdef, params := 0, assume_string := 0, keep_connection := 0);
Debugger(); // put script in debug state

PerformanceMeasure(delta_seconds := 10, max_scripts := 100);

HTTPRequest(url, method := "GET", options := struct{}, flags := 0);
const HTTPREQUEST_EXTENDED_RESPONSE := 0x0001; // return Dictionary with various response data instead of a String of response body

LoadExportedScript(name, args:={});

GetEnvironmentVariable(name:="");
