CXXWARNS = -Wall -pedantic
CXXOPTS  = -g -O3 -I.
CXXFLAGS = $(CXXWARNS) $(CXXOPTS)
PROGRAMS = lambdaminer simulator

lambdaminer: lambdaminer.o
	$(CXX) $(CXXFLAGS) $^ -o $@

simulator: simulator.o
	$(CXX) $(CXXFLAGS) $^ -o $@

all:	$(PROGRAMS)

clean:
	rm -f *.o Makefile.bak

distclean:	clean
	rm -f $(PROGRAMS)

depend:
	makedepend -Y lambdaminer.C simulator.C
