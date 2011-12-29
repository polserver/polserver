
gethostname: gethostname.cpp
	c++ -static -o gethostname gethostname.cpp

dist: gethostname
	tar -cv gethostname gethostname.cpp Makefile | gzip >gethostname.tar.gz

