# ;;; -*- Mode:makefile;-*- 
# Generated automatically from Makefile.in by configure.
# This requires GNU make.

srcdir = .
VPATH = .

# point this to proper location


# PWD is here because SC5 wants absolute path ;(
STL_INCL=-I${PWD}/../../stlport

LIST  = stl_test.cpp accum1.cpp accum2.cpp \
	adjdiff0.cpp adjdiff1.cpp adjdiff2.cpp \
	adjfind0.cpp adjfind1.cpp adjfind2.cpp \
	advance.cpp \
	alg1.cpp alg2.cpp alg3.cpp alg4.cpp alg5.cpp \
	bcompos1.cpp bcompos2.cpp \
	bind1st1.cpp bind1st2.cpp \
	bind2nd1.cpp bind2nd2.cpp \
	binsert1.cpp binsert2.cpp \
	binsrch1.cpp binsrch2.cpp \
	bnegate1.cpp bnegate2.cpp bvec1.cpp \
	copy1.cpp copy2.cpp copy3.cpp copy4.cpp \
	copyb.cpp copyb0.cpp \
	count0.cpp count1.cpp \
	countif1.cpp \
	deque1.cpp \
	divides.cpp \
	eqlrnge0.cpp eqlrnge1.cpp eqlrnge2.cpp \
	equal0.cpp equal1.cpp equal2.cpp \
	equalto.cpp \
	fill1.cpp filln1.cpp \
	find0.cpp find1.cpp \
	findif0.cpp findif1.cpp \
	finsert1.cpp finsert2.cpp \
	foreach0.cpp foreach1.cpp \
	func1.cpp func2.cpp func3.cpp \
	gener1.cpp gener2.cpp \
	genern1.cpp genern2.cpp \
	greateq.cpp greater.cpp \
	incl0.cpp incl1.cpp incl2.cpp \
	inplmrg1.cpp inplmrg2.cpp \
	inrprod0.cpp inrprod1.cpp inrprod2.cpp \
	insert1.cpp insert2.cpp \
	iota1.cpp \
	istmit1.cpp \
	iter1.cpp iter2.cpp iter3.cpp iter4.cpp \
	iterswp0.cpp iterswp1.cpp \
	less.cpp \
	lesseq.cpp \
	lexcmp1.cpp lexcmp2.cpp \
	list1.cpp list2.cpp list3.cpp list4.cpp \
	logicand.cpp logicnot.cpp \
	logicor.cpp \
	lwrbnd1.cpp lwrbnd2.cpp \
	map1.cpp \
	max1.cpp max2.cpp \
	maxelem1.cpp maxelem2.cpp \
	memfunptr.cpp \
	merge0.cpp merge1.cpp merge2.cpp \
	min1.cpp min2.cpp \
	minelem1.cpp minelem2.cpp \
	minus.cpp \
	mismtch0.cpp mismtch1.cpp mismtch2.cpp \
	mkheap0.cpp mkheap1.cpp \
	mmap1.cpp mmap2.cpp \
	modulus.cpp \
	mset1.cpp mset3.cpp mset4.cpp mset5.cpp \
	negate.cpp nequal.cpp \
	nextprm0.cpp nextprm1.cpp nextprm2.cpp \
	nthelem0.cpp nthelem1.cpp nthelem2.cpp \
	ostmit.cpp \
	pair0.cpp pair1.cpp pair2.cpp \
	parsrt0.cpp parsrt1.cpp parsrt2.cpp \
	parsrtc0.cpp parsrtc1.cpp parsrtc2.cpp \
	partsrt0.cpp \
	partsum0.cpp partsum1.cpp partsum2.cpp \
	pheap1.cpp pheap2.cpp \
	plus.cpp \
	pqueue1.cpp \
	prevprm0.cpp prevprm1.cpp prevprm2.cpp \
	ptition0.cpp ptition1.cpp \
	ptrbinf1.cpp ptrbinf2.cpp \
	ptrunf1.cpp ptrunf2.cpp \
	queue1.cpp \
	rawiter.cpp \
	remcopy1.cpp \
	remcpif1.cpp \
	remif1.cpp \
	remove1.cpp \
	repcpif1.cpp \
	replace0.cpp replace1.cpp replcpy1.cpp replif1.cpp \
	revbit1.cpp revbit2.cpp \
	revcopy1.cpp reverse1.cpp reviter1.cpp reviter2.cpp \
	rndshuf0.cpp rndshuf1.cpp rndshuf2.cpp \
	rotate0.cpp rotate1.cpp rotcopy0.cpp rotcopy1.cpp \
	search0.cpp search1.cpp search2.cpp \
	set1.cpp set2.cpp \
	setdiff0.cpp setdiff1.cpp setdiff2.cpp \
	setintr0.cpp setintr1.cpp setintr2.cpp \
	setsymd0.cpp setsymd1.cpp setsymd2.cpp \
	setunon0.cpp setunon1.cpp setunon2.cpp \
	sort1.cpp sort2.cpp \
	stack1.cpp stack2.cpp \
	stblptn0.cpp stblptn1.cpp \
	stblsrt1.cpp stblsrt2.cpp \
	swap1.cpp \
	swprnge1.cpp \
	times.cpp \
	trnsfrm1.cpp trnsfrm2.cpp \
	ucompos1.cpp ucompos2.cpp \
	unegate1.cpp unegate2.cpp \
	uniqcpy1.cpp uniqcpy2.cpp \
	unique1.cpp unique2.cpp \
	uprbnd1.cpp uprbnd2.cpp \
	vec1.cpp vec2.cpp vec3.cpp vec4.cpp vec5.cpp vec6.cpp vec7.cpp vec8.cpp \
        hmap1.cpp hmmap1.cpp hset2.cpp hmset1.cpp slist1.cpp string1.cpp bitset1.cpp

# STAT_MODULE=stat.o
OBJECTS = $(LIST:%.cpp=%.o) $(STAT_MODULE)
EXECS = $(LIST:%.cpp=%.exe)
TESTS = $(LIST:%.cpp=%.out)
TEST_EXE  = stl_test.exe
TEST  = stl_test.out

CC = CC
CXX = $(CC)
# DEBUG_FLAGS=-g
# DEBUG_FLAGS=-O

# DEBUG_FLAGS=-D_STLP_DEBUG

# DEBUG_FLAGS=-compat=4


# CXXFLAGS = -xarch=v9 +w2 ${STL_INCL} ${DEBUG_FLAGS} -I. -D_STLP_NO_OWN_IOSTREAMS -D_STLP_HAS_NO_NEW_IOSTREAMS
CXXFLAGS = -xarch=v9 +w2 ${STL_INCL} ${DEBUG_FLAGS} -I. -library=no%Cstd  -I. -qoption ccfe -expand=1000 -qoption ccfe -instlib=../../lib/libstlport_sunpro64.so



# LIBS = -lm -liostream 
LIBS = -L../../lib -lstlport_sunpro64 -lm
LIBSTDCXX = 

check: $(TEST)

$(TEST) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(OBJECTS) $(LIBS) -o $(TEST_EXE)
	LD_LIBRARY_PATH=../../lib:${LD_LIBRARY_PATH} ./$(TEST_EXE) < stdin > $(TEST)

SUFFIXES: .cpp.o.exe.out.res

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

%.i : %.cpp
	$(CXX) $(CXXFLAGS) $< -E -H > $@

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -c -USINGLE -DMAIN -D_REENTRANT -o $*.o
	$(CXX) $(CXXFLAGS) $*.o $(LIBS) -lpthread -g -o $*.exe
	LD_LIBRARY_PATH=../../lib:${LD_LIBRARY_PATH} ./$*.exe < stdin > $@
	-rm -f $*.exe

istmit1.out: istmit1.cpp
	$(CXX) $(CXXFLAGS) $< $(STAT_MODULE) $(LIBSTDCXX) -lstdc++ $(LIBS) -o istmit1
	echo 'a string' | ./istmit1 > istmit1.out
	-rm -f ./istmit1

$(STAT_MODULE): stat.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.s: %.cpp
	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -noex -D_STLP_NO_EXCEPTIONS -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O4 -noex -D_STLP_NO_EXCEPTIONS -D_STLP_NO_EXCEPTIONS -S -pta $<  -o $@

clean:
	-rm -fr *.exe *.o *.rpo *.obj *.out Templates.DB SunWS_cache
