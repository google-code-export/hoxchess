####################################################################
# The 'makefile' of XQWLight AI Engine.
#
####################################################################

# The name of the App.
LIBRARY = AI_XQWLight

# Common flags
CXX         = g++

CXXFLAGS = -fPIC -Wall -I../common
DEBUGFLAGS  = -g

# The main source
MAIN_SRC := \
	AI_xqwlight.cpp \
	XQWLight.cpp

# Define our sources and object files
SOURCES := \
	$(MAIN_SRC)

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c -o $@ $<

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CXX) -shared -Wl,-soname,lib$(LIBRARY).so.1 -o lib$(LIBRARY).so.1.0 $(OBJECTS)

clean:
	rm -vrf lib$(LIBRARY).* *.o

############## END OF FILE ###############################################

