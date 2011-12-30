#DTYPE=DEBUG
!ifndef DTYPE
DTYPE=RELEASE
!endif

#X64=1

DIFF=..\bcomp\release\bcomp.exe
!ifndef COMPIL32
!ifndef X64
COMPIL32=..\..\pol-core\ecompile\$(DTYPE)\ecompile.exe
!else
COMPIL32=..\..\pol-core\ecompile\$(DTYPE)\x64\ecompile.exe
!endif
!endif

!ifndef X64
EXEC32=..\..\pol-core\runecl\$(DTYPE)\runecl.exe
!else
EXEC32=..\..\pol-core\runecl\$(DTYPE)\x64\runecl.exe
!endif

# Performance testing
PERFS = \
    perfsetup.prf perf001.prf perf002.prf perf003a.prf perf003b.prf \
    perf003.prf perf004.prf\
    perf005a.prf perf005b.prf perf005c.prf \
    perf006.prf perf007.prf

# Experimental scripts
EXPR = 
    

# Tests which should pass the testsuite
OTESTS = \
    dict001.ok dict002.ok dict003.ok dict004.ok dict005.ok dict006.ok dict007.ok \
    dict008.ok dict009.ok dict010.ok dict011.ok dict012.ok dict013.ok dict014.ok \
    struct005.ok struct007.ok struct008.ok struct009.ok \
    math0004.ok math0005.ok math0006.ok \
    export01.ok glob001.ok \
	array05.ok array06.ok array07.ok array08.ok array09.ok array10.ok \
    array11.ok array12.ok array13.ok array15.ok array16.ok array17.ok \
	enum01.ok enum02.ok enum03.ok enum04.ok enum05.ok enum06.ok\
    oper001.ok oper002.ok oper003.ok oper004.ok \
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
    bug001.ok bug002.ok
# bug003.ok bug004.ok

#     
#    ref001.ok ref002.ok switch12.cok 

CTESTS = \
    func06.cok const04.cok var001.cok fcall001.cok func2.cok func2b.cok \
    rpterr.cok funceof.cok comp001.cok scope02.cok scope05.cok bug006.cok \
	bug004.cok inc01.cok array14.cok syntax01.cok syntax02.cok syntax03.cok \
	syntax04.cok syntax05.cok syntax06.cok syntax07.cok syntax09.cok crash001.cok

FAIL=label.ok

test:	copyem otests ctests
	@echo All tests passed.

copyem:
!	ifndef X64
	@copy ..\..\lib\mysql-connector-c-6.0.2-win32\lib\libmysql.dll .
!	else
	@copy ..\..\lib\mysql-connector-c-6.0.2-winx64\lib\libmysql.dll .
!	endif
	@copy ..\..\pol-core\support\scripts\*.em .
	@copy ..\..\pol-core\support\scripts\ecompile.cfg.example ecompile.cfg
		
otests: $(OTESTS)

ctests: $(CTESTS)

clean:
    del *.ok
    del *.cok
    del *.em
    del *.ecl
    del *.lst
    del *.dep
    del *.dbg
    del *.dbg.txt
    del *.tst
    del *.prf
    del *.tsi
    del *.ctt
    del libmysql.dll
    del ecompile.cfg

perf: copyem perf.out

perf.out: $(PERFS)
    @echo perfsetup >perf.out
    @type perfsetup.prf >>perf.out
    @echo perf001.prf >>perf.out
    @type perf001.prf >>perf.out
    @echo perf002.prf >>perf.out
    @type perf002.prf >>perf.out
    @echo perf003a.prf >>perf.out
    @type perf003a.prf >>perf.out
    @echo perf003b.prf >>perf.out
    @type perf003b.prf >>perf.out
    @echo perf003.prf >>perf.out
    @type perf003.prf >>perf.out
    @echo perf004.prf >>perf.out
    @type perf004.prf >>perf.out
    @echo perf005a.prf >>perf.out
    @type perf005a.prf >>perf.out
    @echo perf005b.prf >>perf.out
    @type perf005b.prf >>perf.out
    @echo perf005c.prf >>perf.out
    @type perf005c.prf >>perf.out
    @echo perf006.prf >>perf.out
    @type perf006.prf >>perf.out
    @echo perf007.prf >>perf.out
    @type perf007.prf >>perf.out

expr: $(EXPR)

help:
    @echo "Build configurations:"
    @echo "    test      entire test suite (default)"
    @echo "    perf      performance tests"
    @echo "    expr      experimental scripts"
    @echo "    help      this list"
    @echo "    clean     clean for new test run"

.SUFFIXES : .src .out .prf

# Main Rule: 
#  A .SRC file is compiled into a .ECL file.
#  The .ECL file is executed, into a .TST file
#  Expected results are already in a .OUT file, which the .TST should match.
#  If all goes well, a .OK file is created.

# TODO: run a non-optimized compile/run as well as an optimized compile/run
#       otherwise, we risk not testing everything.
.src.ok:
	@echo $(@B).src: TESTING
	@del $(@B).ok 2>NUL
	@del $(@B).ecl 2>NUL
    @del $(@B).tst 2>NUL
    @del $(@B).tsi 2>NUL
	$(COMPIL32) -l -xt -q $(@B).src
    $(EXEC32) -q -v$(@B).ecl >$(@B).tsi
	$(EXEC32) -q $(@B).ecl >$(@B).tst
    $(EXEC32) -p -q $(@B).ecl >$(@B).prf
	$(DIFF) $(@B).out $(@B).tst
	@echo $(@B).src: output PASSED
	@echo $(@B).src: instructions TESTING
    @REM copy $(@B).tsi $(@B).ins 
    -$(DIFF) $(@B).ins $(@B).tsi
    @echo $(@B).src: instructions PASSED
	@echo $(@B).src: PASSED >$(@B).ok

.src.prf:
	@echo $(@B).src: PROFILING
	@del $(@B).prf 2>NUL
	@del $(@B).ecl 2>NUL
	@$(COMPIL32) -l -q $(@B).src
    @$(EXEC32) -p -q $(@B).ecl >$(@B).prf
	@echo $(@B).src: COMPLETE

# For testing Compiler output:
# A .SRC file is compiled, and the compiler stdout output is examined.
.src.cok:
	@echo TESTING COMPILER OUTPUT: $(@B).src
	@del $(@B).ok >NUL
	@del $(@B).ecl >NUL
	@$(COMPIL32) -e -q $(@B).src >$(@B).ctt 
	@-$(DIFF) $(@B).cot $(@B).ctt
	@echo $(@B).src: PASSED >$(@B).cok
	@echo $(@B).src: PASSED



