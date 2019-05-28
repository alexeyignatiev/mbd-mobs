### Makefile ---

## Author: Alexey S. Ignatiev
## Keywords:

# main parameters
CXX      := c++
CXXFLAGS := -std=c++11 -fPIC -Wall -Wno-deprecated -fno-strict-aliasing
CXXOPT   := -O3 -DNDEBUG
CXXDEBUG := -O0 -g3
CXXPROF  := -O3 -g3 -fno-inline -fno-omit-frame-pointer -pg -DNDEBUG
INCLUDES := -I. -I../include
LIBS     := -L.
HEADERS  :=  id_generator.hh id_manager.hh vid_manager.hh
SOURCES  :=  vid_manager.cc
OBJECTS  := $(SOURCES:.cc=.o)
TRGT     := idman

# gather compiler options
CXXARGS := $(CXXFLAGS) $(INCLUDES) $(CXXOPT)

# targets
.PHONY: all lib

all: lib

lib: $(OBJECTS)
	@-ar cr lib$(TRGT).a $(OBJECTS)

%.o: %.cc
	$(CXX) $(CXXARGS) -c $< -o $@

clean:
	@-find . -name '*.o' -delete
	@-find . -name '*.so' -delete
	@-find . -name '*.a' -delete

#-------------------------------------------------------------------------------
