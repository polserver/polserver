' This script is used to request admin privileges from the user before
' running hook_helper.bat. This is because Windows requires admin privileges
' for creating symbolic links and batch files can't escalate their own
' permissions.

set shellObj = CreateObject("Shell.Application")
set fso = CreateObject("Scripting.FileSystemObject")
dim currentFolder
currentFolder = fso.GetAbsolutePathName(".")

shellObj.ShellExecute "cmd", "/C cd /d """ & currentFolder & """ && hook_helper.bat", "", "runas", 1