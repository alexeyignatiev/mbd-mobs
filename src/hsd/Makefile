### Makefile ---

## Author: Alexey S. Ignatiev
## Keywords:

# main parameters
CXX      := c++
CXXFLAGS := -std=c++11 -fPIC -Wall -Wno-deprecated -fno-strict-aliasing
CXXOPT   := -O3 -DNDEBUG
CXXDEBUG := -O0 -g3
CXXPROF  := -O3 -g3 -fno-inline -fno-omit-frame-pointer -pg -DNDEBUG
INCLUDES := -I. -Ideps/hitman -Ideps/hitman/sat/inc -Ideps/idman -Ideps/include
LIBS     := -L.
HEADERS  :=  coex_finder.hh options.hh wcnffmt-mf.hh
SOURCES  :=  coex_finder.cc hsd.cc options.cc
OBJECTS  := $(SOURCES:.cc=.o)
TRGT     :=  hsd

# gather compiler options
CXXARGS := $(CXXFLAGS) $(INCLUDES) $(CXXOPT)

# targets
.PHONY: all build clean debug profile release lib

all: hsd

hsd: $(OBJECTS) hitman idman
	$(CXX) $(CXXARGS) $(INCLUDES) $(LIBS) -o $(TRGT) $(OBJECTS) -lhitman -lidman -lm -lz

%.o: %.cc
	$(CXX) $(CXXARGS) -c $< -o $@

hitman:
	@-(cd deps/hitman; make; cp libhitman.a ../..; cd ../..)

idman:
	@-(cd deps/idman; make; cp libidman.a ../..; cd ../..)

clean:
	@-find . -name '$(TRGT)' -delete
	@-find . -name '*.o' -delete
	@-find . -name '*.so' -delete
	@-find . -name '*.a' -delete

#-------------------------------------------------------------------------------
