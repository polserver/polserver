###############################################################################
#
# Copyright (C) 2015 Thomas Volkert <thomas@homer-conferencing.com>
# All rights reserved.
# 
# The environment was provided for the POL project. Redistribution 
# and use in source and binary forms, with or without modification, 
# are permitted. However, this paragraph has to be included in its 
# original form in every project.
#
###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
	../../lib/tinyxml/tinystr.cpp 
	../../lib/tinyxml/tinyxml.cpp
	../../lib/tinyxml/tinyxmlerror.cpp 
	../../lib/tinyxml/tinyxmlparser.cpp
	../../lib/format/format.cc
	accounts/account.cpp 
	accounts/accounts.cpp 
	accounts/acscrobj.cpp 
	allocd.cpp
	guilds.cpp
	item/armor.cpp 
	mobile/attack.cpp 
	mobile/ufacing.cpp
	mobile/attribute.cpp 
	network/auxclient.cpp 
	network/packets.cpp
	network/bannedips.cpp 
	binaryfilescrobj.cpp 
	multi/boat.cpp 
	multi/boatcomp.cpp
	module/boatmod.cpp 
	mobile/boundbox.cpp 
	bowsalut.cpp
	module/cfgmod.cpp 
	cfgrepos.cpp 
	network/cgdata.cpp 
	mobile/charactr.cpp
	checkpnt.cpp
	mobile/chrcast.cpp 
	mobile/chrituse.cpp 
	mobile/chrskuse.cpp 
	network/client.cpp
	network/clientio.cpp 
	network/clienttransmit.cpp
	network/cliface.cpp 
	cmbtcfg.cpp 
	cmdlevel.cpp 
	containr.cpp
	console.cpp 
	core.cpp 
	mobile/corpse.cpp 
	mobile/wornitems.cpp
	create.cpp
	crypt/cryptengine.cpp 
	crypt/cryptbase.cpp 
	crypt/cryptkey.cpp
	crypt/blowfish.cpp 
	crypt/twofish.cpp 
	crypt/crypt.cpp 
	crypt/logincrypt.cpp 
	crypt/md5.cpp
	ctable.cpp
	multi/customhouses.cpp 
	multi/customhousehelp.cpp
	module/datastore.cpp 
	dblclick.cpp 
	decay.cpp 
	dice.cpp
	door.cpp 
	dropitem.cpp
	eqpitem.cpp 
	equipdsc.cpp 
	item/equipmnt.cpp 
	exscrobj.cpp
	extobj.cpp
	module/filemod.cpp 
	fnsearch.cpp
	gameclck.cpp 
	getitem.cpp 
	getmsg.cpp 
	guardrgn.cpp
	module/guildmod.cpp
	module/basiciomod.cpp 
	module/basicmod.cpp
	help.cpp 
	multi/house.cpp
	network/iostats.cpp 
	irequest.cpp 
	item/item.cpp 
	item/item00.cpp
	item/itemcr.cpp 
	item/itemdesc.cpp
	landtile.cpp
	listenpt.cpp 
	loadunld.cpp 
	lockable.cpp
	loaddata.cpp 
	login.cpp
	module/mathmod.cpp 
	menu.cpp 
	menusel.cpp 
	miscmsg.cpp
	miscrgn.cpp 
	mkscrobj.cpp
	sqlscrobj.cpp
	movecost.cpp 
	movement.cpp 
	network/msgfiltr.cpp 
	network/msghandl.cpp
	multi/multicfg.cpp 
	multi/multicr.cpp 
	multi/multidef.cpp 
	multi/multis.cpp
	multi/multidef2.cpp
	musicrgn.cpp
	npctmpl.cpp 
	npctemplates.cpp 
	module/npcmod.cpp
	objecthash.cpp 
	module/osmod.cpp
	network/packethooks.cpp 
	packetscrobj.cpp 
	party.cpp 
	module/partymod.cpp
	pol.cpp 
	polcfg.cpp 
	polclock.cpp 
	poldbg.cpp
	polfile2.cpp 
	polsem.cpp 
	polsig.cpp 
	polstats.cpp
	module/polsystemmod.cpp
	poltest.cpp 
	polwww.cpp 
	module/httpmod.cpp 
	proplist.cpp
	reftypes.cpp 
	region.cpp 
	repsys.cpp
	resource.cpp
	realms.cpp
	savedata.cpp
	schedule.cpp 
	scrdef.cpp 
	scrsched.cpp 
	scrstore.cpp
	skilladv.cpp
	sngclick.cpp 
	sockio.cpp 
	speech.cpp 
	spelbook.cpp
	spells.cpp 
	ssopt.cpp
	stackcfg.cpp
	startloc.cpp 
	statmsg.cpp 
	storage.cpp 
	syshook.cpp
	syshookscript.cpp
	target.cpp 
	tasks.cpp 
	textcmd.cpp
	tildecmd.cpp 
	tiles.cpp 
	tiplstwn.cpp
	tips.cpp 
	tmpvars.cpp 
	tooltips.cpp
	ufunc.cpp 
	ufunc2.cpp 
	uimport.cpp
	umap.cpp 
	module/unimod.cpp
	uobjcnt.cpp 
	uobject.cpp 
	uobjhelp.cpp 
	uoclient.cpp
	module/uomod.cpp 
	module/uomod2.cpp
	module/uomod3.cpp 
	module/uomod4.cpp 
	uoexhelp.cpp
	uolisten.cpp
	uoscrobj.cpp 
	uoskills.cpp 
	useskill.cpp
	module/utilmod.cpp 
	module/sqlmod.cpp
	mobile/npc.cpp
	uworld.cpp
	vital.cpp
	item/weapon.cpp
	zone.cpp
	module/attributemod.cpp 
	module/clmod.cpp 
	clfunc.cpp 
	uoexec.cpp
	globals/uvars.cpp
	globals/ucfg.cpp 
	globals/multidefs.cpp
	globals/network.cpp 
	globals/script_internals.cpp 
	globals/settings.cpp
	globals/state.cpp
	globals/object_storage.cpp
	globals/memoryusage.cpp
	uworld.cpp 
	vital.cpp	
	module/storagemod.cpp 
	module/vitalmod.cpp
	network/clientthread.cpp
	xmlfilescrobj.cpp
	network/packetdefs.cpp
)	

##############################################################
# include dirs
SET (INCLUDE_DIRS
	../
)

##############################################################
# set directory for internal libs
SET (LIB_DIRS
	${LIB_DIRS}
	${CMAKE_CURRENT_LIST_DIR}/../bin
)
	
##############################################################
# compile flags
IF(NOT WINDOWS)
	SET (FLAGS
		${FLAGS}
		-fopenmp  
		-DINC_PASSERT=1 
		-DTIXML_USE_STL 
		-Wall 
	    -Wno-uninitialized  #TODO: remove and fix the code
    	-Wno-ignored-qualifiers #TODO: remove and fix the code
	)
ENDIF()

##############################################################
# link to the needed libs
SET (LIBS_LINUX
	bscript
	clib_debugging
	clib
	plib
	crypto
    pthread
    z
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_PROGRAM_NAME
	pol
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
