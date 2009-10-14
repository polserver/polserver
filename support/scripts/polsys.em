// Core 95 required to use this module

const MSGLEN_VARIABLE := -1;

AddRealm(realm_name,base_realm);
CreatePacket(type,size);
DeleteRealm(realm);
GetCmdLevelName(number);
GetCmdLevelNumber(name);
GetItemDescriptor(objtype);
GetPackageByName(name);
IncRevision(object);
ListTextCommands();
ListenPoints();
Packages();
ReadMillisecondClock();
Realms(realm:="");
ReloadConfiguration(); // reloads pol.cfg and npcdesc.cfg
SetSysTrayPopupText(text);
