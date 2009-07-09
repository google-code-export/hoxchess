####################################################################
# The 'Makefile' of Folium AI Engine.
#
####################################################################

# The name of the App.
LIBRARY = AI_Folium

# Common flags
CXX         = g++

CXXFLAGS = -fPIC -Wall -I../common
#DEBUGFLAGS  = -g

# The main source
MAIN_SRC := \
	AI_Folium.cpp \
	bitmap_data.cpp \
	engine.cpp \
	folHOXEngine.cpp \
	generator.cpp \
	hash.cpp \
	history_data.cpp \
	move_helper.cpp \
	search.cpp \
	xq.cpp \
	xq_data.cpp \
	xq_helper.cpp \
	xq_position_data.cpp \
	utility/str.cpp \
	utility/time.cpp \

# Define our sources and object files
SOURCES := \
	$(MAIN_SRC)

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c -o $@ $<

all: $(LIBRARY)
	cp -v libAI_Folium.so.1.0 ../AI_Folium.so

$(LIBRARY): $(OBJECTS)
	$(CXX) -shared -Wl,-soname,lib$(LIBRARY).so.1 -o lib$(LIBRARY).so.1.0 $(OBJECTS)

clean:
	rm -vrf lib$(LIBRARY).* *.o

############## END OF FILE ###############################################

