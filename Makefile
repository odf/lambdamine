CXXWARNS = -Wall -pedantic
CXXOPTS  = -g -O3 -I.
CXXFLAGS = $(CXXWARNS) $(CXXOPTS)
PROGRAMS = lambdaminer simulator

lambdaminer: lambdaminer.o Game.o
	$(CXX) $(CXXFLAGS) $^ -o $@

simulator: simulator.o Game.o
	$(CXX) $(CXXFLAGS) $^ -o $@

all:	$(PROGRAMS)

clean:
	rm -f *.o Makefile.bak

distclean:	clean
	rm -f $(PROGRAMS)

depend:
	makedepend -Y Game.C lambdaminer.C simulator.C
# DO NOT DELETE

Game.o: Game.h
simulator.o: Game.h
