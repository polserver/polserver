// Core 95 required to use this module

const MSGLEN_VARIABLE := -1;

AddRealm(realm_name,base_realm);
CreatePacket(type,size);
DeleteRealm(realm);
GetCmdLevelName(number);
GetCmdLevelNumber(name);
GetItemDescriptor(objtype);
FormatItemDescription(desc, amount := 1, suffix := "");
GetPackageByName(name);
IncRevision(object);
ListTextCommands(max_cmdlevel := -1); // the default (-1) returns all
ListenPoints();
MD5Encrypt(str);
Packages();
ReadMillisecondClock();
Realms(realm:="");
ReloadConfiguration(); // reloads pol.cfg and npcdesc.cfg
SetSysTrayPopupText(text);
LogCPropProfile();
