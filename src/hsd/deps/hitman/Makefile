### Makefile ---

## Author: Alexey S. Ignatiev
## Keywords:

# main parameters
CXX      := c++
CXXFLAGS := -std=c++11 -fPIC -Wall -Wno-deprecated -fno-strict-aliasing
CXXOPT   := -O3 -DNDEBUG
CXXDEBUG := -O0 -g3
CXXPROF  := -O3 -g3 -fno-inline -fno-omit-frame-pointer -pg -DNDEBUG
INCLUDES := -I. -Isat/inc
LIBS     := -L.
HEADERS  :=  hitman.hh hit_minimal.hh hit_minimum.hh
SOURCES  :=  hit_minimal.cc hit_minimum.cc
OBJECTS  := $(SOURCES:.cc=.o)
TRGT     :=  hitman

# gather compiler options
CXXARGS := $(CXXFLAGS) $(INCLUDES) $(CXXOPT)

# targets
.PHONY: all build clean debug profile release lib

all: lib

lib: $(OBJECTS) satobjs
	@-ar cr lib$(TRGT).a $(OBJECTS) sat/*.o

%.o: %.cc
	$(CXX) $(CXXARGS) -c $< -o $@

satobjs:
	@-(cd sat; make obj; cd ..)

clean:
	@-find . -name '*.o' -delete
	@-find . -name '*.so' -delete
	@-find . -name '*.a' -delete

ex: lib
	$(CXX) $(CXXARGS) $(INCLUDES) $(LIBS) -o example example.cc -l$(TRGT)

#-------------------------------------------------------------------------------
