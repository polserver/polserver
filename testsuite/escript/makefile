#DTYPE=DEBUG
!ifndef DTYPE
DTYPE=RELEASE
!endif

#X64=1

DIFF=..\bcomp\release\bcomp.exe
COMPIL32=..\..\pol-core\ecompile\$(DTYPE)\ecompile.exe
COMPIL64=..\..\pol-core\ecompile\$(DTYPE)\x64\ecompile.exe
!ifndef COMPIL
!   ifndef X64
COMPIL=$(COMPIL32)
!   else
COMPIL=$(COMPIL64)
!   endif
!endif

EXEC32=..\..\pol-core\runecl\$(DTYPE)\runecl.exe
EXEC64=..\..\pol-core\runecl\$(DTYPE)\x64\runecl.exe
!ifndef X64
EXEC=$(EXEC32)
!else
EXEC=$(EXEC64)
!endif

# Performance testing
# *f.prf += versions of the scripts should always be faster
PERFS = \
    perfsetup.prf \
    perf001.prf perf001f.prf \
    perf002.prf perf002f.prf \
    perf003a.prf perf003af.prf \
    perf003b.prf perf003bf.prf \
    perf003.prf perf003f.prf\
    perf004.prf perf004f.prf\
    perf005a.prf perf005b.prf perf005c.prf \
    perf006.prf perf007.prf\

# Experimental scripts
EXPR =


# Tests which should pass the testsuite
!ifndef OTESTS
OTESTS = \
    dict001.ok dict002.ok dict003.ok dict004.ok dict005.ok dict006.ok dict007.ok \
    dict008.ok dict009.ok dict010.ok dict011.ok dict012.ok dict013.ok dict014.ok dict015.ok \
    struct005.ok struct007.ok struct008.ok struct009.ok struct011.ok \
    math0004.ok math0005.ok math0006.ok \
    export01.ok glob001.ok \
    array05.ok array06.ok array07.ok array08.ok array09.ok array10.ok \
    array11.ok array12.ok array13.ok array15.ok array16.ok array17.ok array18.ok \
    enum01.ok enum02.ok enum03.ok enum04.ok enum05.ok enum06.ok\
    oper001.ok oper002.ok oper003.ok oper004.ok oper005.ok oper006.ok oper007.ok \
    prog01.ok prog02.ok prog03.ok \
    prec01.ok prec02.ok prec03.ok \
    while13.ok \
    switch01.ok switch02.ok switch03.ok switch04.ok switch05.ok \
    switch06.ok switch07.ok switch08.ok switch09.ok switch10.ok \
    switch11.ok switch12.ok switch13.ok switch14.ok \
    foreach01.ok foreach02.ok foreach03.ok foreach04.ok foreach05.ok \
    foreach07.ok foreach08.ok foreach09.ok foreach10.ok foreach11.ok \
    foreach06.ok \
    func0b.ok func0ab.ok func1b.ok func04.ok func05.ok \
    func07.ok \
    for001b.ok for002b.ok for003b.ok for004b.ok \
    for005.ok for006.ok for007.ok for008.ok for009.ok for010.ok for011.ok for012.ok \
    repeat01.ok repeat02.ok repeat03.ok repeat04.ok repeat05.ok repeat06.ok \
    repeat07.ok repeat08.ok repeat09.ok repeat10.ok  \
    while01.ok while02.ok while03.ok while04.ok while05.ok while06.ok while07.ok\
    while08.ok while09.ok while10.ok while11.ok \
    if.ok if01.ok ifthen.ok \
    bracket.ok bracket1.ok bracket02.ok \
    func0.ok func0a.ok func1.ok \
    array01.ok array02.ok array03.ok array04.ok \
    assign.ok do0.ok convert.ok \
    ts.ok test.ok test2.ok test3.ok test6.ok struct.ok struct002.ok \
    struct003.ok struct004.ok \
    long001.ok not.ok\
    quot001.ok \
    for001.ok for002.ok for003.ok for004.ok \
    substr01.ok substr02.ok substr03.ok substr04.ok \
    string17.ok string16.ok string15.ok string14.ok \
    string13.ok string01.ok string02.ok string03.ok string04.ok string05.ok string06.ok \
    string07.ok string08.ok string09.ok string10.ok string11.ok string12.ok \
    error01.ok inctst01.ok hello2.ok \
    const01.ok const02.ok const03.ok  \
    in001.ok in002.ok in003.ok in004.ok \
    inc02.ok scope01.ok scope03.ok scope04.ok \
    var01.ok var02.ok var03.ok \
    method01.ok \
    narray1.ok narray2.ok narray3.ok narray4.ok narray5.ok \
    narray6.ok narray7.ok narray8.ok narray9.ok narray10.ok \
    narray11.ok narray12.ok narray13.ok narray14.ok narray15.ok \
    narray16.ok narray17.ok narray18.ok \
    pack01.ok pack02.ok pack03.ok pack04.ok pack05.ok pack06.ok pack07.ok \
    pack08.ok \
    opt001.ok opt002.ok opt003.ok opt004.ok \
    bfor001.ok bfor002.ok bfor003.ok bfor004.ok bfor005.ok \
    reff001.ok reff002.ok reff003.ok \
    op001.ok \
    cint001.ok meth001.ok size001.ok \
    math0001.ok math0002.ok math0003.ok \
    mod0001.ok \
    bitop001.ok bitop002.ok bitop003.ok \
    recurs01.ok recurs02.ok \
    bug001.ok bug002.ok bug003.ok \
    setmember.ok \
    plusequal.ok \
    filemod\xml.ok
!endif
#  bug004.ok

#
#    ref001.ok ref002.ok switch12.cok

!ifndef CTESTS
CTESTS = \
    func06.cok const04.cok var001.cok fcall001.cok func2.cok func2b.cok \
    rpterr.cok funceof.cok comp001.cok scope02.cok scope05.cok bug006.cok \
    bug004.cok inc01.cok array14.cok syntax01.cok syntax02.cok syntax03.cok \
    syntax04.cok syntax05.cok syntax06.cok syntax07.cok syntax09.cok crash001.cok
!endif
FAIL=label.ok

test:   copy otests ctests
    @echo All tests passed.

copy:
!   ifndef X64
        @copy ..\..\lib\mysql-connector-c-6.0.2-win32\lib\libmysql.dll .
!   else
        @copy ..\..\lib\mysql-connector-c-6.0.2-winx64\lib\libmysql.dll .
!   endif

otests: $(OTESTS)

ctests: $(CTESTS)

clean:
    @del /S *.ok 2>NUL
    @del /S *.cok 2>NUL
    @del /S *.ecl 2>NUL
    @del /S *.ecl32 2>NUL
    @del /S *.lst 2>NUL
    @del /S *.dep 2>NUL
    @del /S *.dbg 2>NUL
    @del /S *.dbg.txt 2>NUL
    @del /S *.tst 2>NUL
    @del /S *.prf 2>NUL
    @del /S *.tsi 2>NUL
    @del /S *.ctt 2>NUL
    @del libmysql.dll 2>NUL

perf: copy perfout $(PERFS)

perfout:
!   ifdef X64
    @echo X64 >perf.out
!   else
    @echo Win32 >perf.out
!   endif

expr: $(EXPR)

create: copy
!   ifdef FILE
        @del $(FILE).ecl 2>NUL
        @del $(FILE).ins 2>NUL
        @del $(FILE).out 2>NUL
        $(COMPIL) -l -xt -q $(FILE).src
        $(EXEC) -q -v$(FILE).ecl >$(FILE).ins
        $(EXEC) -q $(FILE).ecl >$(FILE).out
        @del $(FILE).ecl 2>NUL
        @del $(FILE).dbg 2>NUL
        @del $(FILE).dbg.txt 2>NUL
        @del $(FILE).dep 2>NUL
        @del $(FILE).lst 2>NUL
!   else
        @echo you need to define FILE
!   endif

help:
    @echo "Build configurations:"
    @echo "    test      entire test suite (default)"
    @echo "    perf      performance tests"
    @echo "    expr      experimental scripts"
    @echo "    help      this list"
    @echo "    clean     clean for new test run"
    @echo "    create    creates ref out,ins files for given FILE
    @echo "    BINCOMP=1 use this parameter to compare 64bit ecompile output against 32bit

.SUFFIXES : .src .out .prf

# Main Rule:
#  A .SRC file is compiled into a .ECL file.
#  The .ECL file is executed, into a .TST file
#  Expected results are already in a .OUT file, which the .TST should match.
#  If all goes well, a .OK file is created.

# TODO: run a non-optimized compile/run as well as an optimized compile/run
#       otherwise, we risk not testing everything.
.src.ok:
    @echo $(@R).src: TESTING
    @del $(@R).ok 2>NUL
    @del $(@R).ecl 2>NUL
    @del $(@R).tst 2>NUL
    @del $(@R).tsi 2>NUL
!ifndef BINCOMP
    $(COMPIL) -l -xt -q $(@R).src
    $(EXEC) -q -v$(@R).ecl >$(@R).tsi
    $(EXEC) -q $(@R).ecl >$(@R).tst
    $(EXEC) -p -q $(@R).ecl >$(@R).prf
    $(DIFF) $(@R).out $(@R).tst
    @echo $(@R).src: output PASSED
    @echo $(@R).src: instructions TESTING
    @REM copy $(@R).tsi $(@R).ins
    -$(DIFF) $(@R).ins $(@R).tsi
    @echo $(@R).src: instructions PASSED
    @echo $(@R).src: PASSED >$(@R).ok
!else
    $(COMPIL32) -l -xt -q $(@R).src
    @move $(@R).ecl $(@R).ecl32
    $(COMPIL64) -l -xt -q $(@R).src
    $(DIFF) $(@R).ecl32 $(@R).ecl
    @echo $(@R).src: output PASSED
!endif

.src.prf:
    @echo $(@R).src: PROFILING
    @del $(@R).prf 2>NUL
    @del $(@R).ecl 2>NUL
    @$(COMPIL) -l -q $(@R).src
    @$(EXEC) -p -q $(@R).ecl >$(@R).prf
    @echo $(@R).src: COMPLETE
    @echo $(@R).prf >>perf.out
    @type $(@R).prf >>perf.out


# For testing Compiler output:
# A .SRC file is compiled, and the compiler stdout output is examined.
.src.cok:
    @echo TESTING COMPILER OUTPUT: $(@R).src
    @del $(@R).ok 2>NUL
    @del $(@R).ecl 2>NUL
    @$(COMPIL) -e -q $(@R).src >$(@R).ctt
    @-$(DIFF) $(@R).cot $(@R).ctt
    @echo $(@R).src: PASSED >$(@R).cok
    @echo $(@R).src: PASSED



