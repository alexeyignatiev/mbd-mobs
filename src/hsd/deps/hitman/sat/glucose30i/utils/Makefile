### Makefile ---

## Author: aign
## Keywords:

# main parameters
CXX      := c++
CXXFLAGS := -std=c++11 -fPIC -Wall -Wno-deprecated -fno-strict-aliasing -O3 -DNDEBUG
INCLUDES := -I../inc
LIBS     :=
SOURCES  :=  options.cc system.cc
OBJECTS  := $(SOURCES:.cc=.o)
TRGT     :=  glucose30i

obj: $(OBJECTS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

#-------------------------------------------------------------------------------

### Makefile ends here
