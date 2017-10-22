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

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("########################################################################")
MESSAGE("##### Building POL ${BINARY_VERSION} (${BINARY_VERSION_NAME})")
MESSAGE("########################################################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
    accounts/account.cpp 
    accounts/accounts.cpp 
    accounts/acscrobj.cpp 
    allocd.cpp
    binaryfilescrobj.cpp 
    bowsalut.cpp
    cfgrepos.cpp 
    checkpnt.cpp
    clfunc.cpp 
    cmbtcfg.cpp 
    cmdlevel.cpp 
    console.cpp 
    containr.cpp
    core.cpp 
    create.cpp
    crypt/blowfish.cpp 
    crypt/crypt.cpp 
    crypt/cryptbase.cpp 
    crypt/cryptengine.cpp 
    crypt/cryptkey.cpp
    crypt/logincrypt.cpp 
    crypt/md5.cpp
    crypt/twofish.cpp 
    ctable.cpp
    dblclick.cpp 
    decay.cpp 
    dice.cpp
    door.cpp 
    dropitem.cpp
	dynproperties.cpp
    eqpitem.cpp 
    equipdsc.cpp 
    exscrobj.cpp
    extobj.cpp
    fnsearch.cpp
    gameclck.cpp 
    getitem.cpp 
    getmsg.cpp 
    globals/memoryusage.cpp
    globals/multidefs.cpp
    globals/network.cpp 
    globals/object_storage.cpp
    globals/script_internals.cpp 
    globals/settings.cpp
    globals/state.cpp
    globals/ucfg.cpp 
    globals/uvars.cpp
    guardrgn.cpp
    guilds.cpp
    help.cpp 
    irequest.cpp 
    item/armor.cpp 
    item/equipmnt.cpp 
    item/item.cpp 
    item/item00.cpp
    item/itemcr.cpp 
    item/itemdesc.cpp
    item/weapon.cpp
    landtile.cpp
    listenpt.cpp 
    loaddata.cpp 
    loadunld.cpp 
    lockable.cpp
    login.cpp
    menu.cpp 
    menusel.cpp 
    miscmsg.cpp
    miscrgn.cpp 
    mkscrobj.cpp
    mobile/attack.cpp 
    mobile/attribute.cpp 
    mobile/boundbox.cpp 
    mobile/charactr.cpp
    mobile/chrcast.cpp 
    mobile/chrituse.cpp 
    mobile/chrskuse.cpp 
    mobile/corpse.cpp 
    mobile/npc.cpp
    mobile/ufacing.cpp
    mobile/wornitems.cpp
    module/attributemod.cpp 
    module/basiciomod.cpp 
    module/basicmod.cpp
    module/boatmod.cpp 
    module/cfgmod.cpp 
    module/clmod.cpp 
    module/datastore.cpp 
    module/filemod.cpp 
    module/guildmod.cpp
    module/httpmod.cpp 
    module/mathmod.cpp 
    module/npcmod.cpp
    module/osmod.cpp
    module/partymod.cpp
    module/polsystemmod.cpp
    module/sqlmod.cpp
    module/storagemod.cpp 
    module/unimod.cpp
    module/uomod.cpp 
    module/uomod2.cpp
    module/uomod3.cpp 
    module/uomod4.cpp 
    module/utilmod.cpp 
    module/vitalmod.cpp
    movecost.cpp 
    movement.cpp 
    multi/boat.cpp 
    multi/boatcomp.cpp
    multi/customhousehelp.cpp
    multi/customhouses.cpp 
    multi/house.cpp
    multi/multicfg.cpp 
    multi/multicr.cpp 
    multi/multidef.cpp 
    multi/multidef2.cpp
    multi/multis.cpp
    musicrgn.cpp
    network/auxclient.cpp 
    network/bannedips.cpp 
    network/cgdata.cpp 
    network/client.cpp
    network/clientio.cpp 
    network/clientthread.cpp
    network/clienttransmit.cpp
    network/cliface.cpp 
    network/iostats.cpp 
    network/msgfiltr.cpp 
    network/msghandl.cpp
    network/packetdefs.cpp
    network/packethooks.cpp 
    network/packets.cpp
    npctemplates.cpp 
    npctmpl.cpp 
    objecthash.cpp 
    packetscrobj.cpp 
    party.cpp 
    pol.cpp 
    polcfg.cpp 
    polclock.cpp 
    poldbg.cpp
    polfile2.cpp 
    PolMain.cpp
    polsem.cpp 
    polsig.cpp 
    polstats.cpp
    polwww.cpp 
    proplist.cpp
    realms.cpp
    realms/realm.cpp
    realms/realmfunc.cpp
    realms/realmlos.cpp 
    reftypes.cpp 
    region.cpp 
    repsys.cpp
    resource.cpp
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
    sqlscrobj.cpp
    ssopt.cpp
    stackcfg.cpp
    startloc.cpp 
    statmsg.cpp 
    storage.cpp 
    syshook.cpp
    syshookscript.cpp
    target.cpp 
    tasks.cpp
    testing/poltest.cpp
    testing/testdrop.cpp
    testing/testenv.cpp 
    testing/testlos.cpp
    testing/testmisc.cpp
    testing/testskill.cpp
    testing/testwalk.cpp
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
    unicode.cpp
    uobjcnt.cpp 
    uobject.cpp 
    uoclient.cpp
    uoexec.cpp
    uoexhelp.cpp
    uolisten.cpp
    uoscrobj.cpp 
    uoskills.cpp 
    useskill.cpp
    uworld.cpp
    uworld.cpp 
    vital.cpp
    vital.cpp    
    xmlfilescrobj.cpp
    zone.cpp
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
        -Wno-uninitialized  #TODO: remove and fix the code
        -Wno-ignored-qualifiers #TODO: remove and fix the code
    )
    IF(GCC)
		SET (FLAGS
			${FLAGS}
			-Wl,-R.
			-Wl,-R./lib
			-Wl,-R../lib
			-Wl,-R../../lib
		)
	ENDIF()    
ENDIF()

##############################################################
# link to the needed libs
SET (LIBS_LINUX
    bscript
    clib
    plib
    tinyxml
    crypto
    pthread
    z
)
IF(HAVE_MYSQL)
	SET (LIBS_LINUX
		${LIBS_LINUX}
		mysqlclient
	)
	IF(DEFINED USE_BOOST_REGEX)
		SET (LIBS_LINUX
			${LIBS_LINUX}
			boost_regex
		)
	ENDIF()
ENDIF()
SET (LIBS_LINUX_STATIC_BIN_STATIC
    bscript_static
    clib_static
    plib_static
    tinyxml_static
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
