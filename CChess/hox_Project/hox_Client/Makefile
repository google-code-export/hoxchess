# Your project's name.
PROGRAM = hoxchess

# Common flags
CXX         = g++
WX_CXXFLAGS = `wx-config --cxxflags`
WX_LDLIBS   = `wx-config --libs`
LDFLAGS     = -pthread

# Define our sources and object files
SOURCES := \
	hoxBoard.cpp \
	hoxConnection.cpp \
	hoxCoreBoard.cpp \
	hoxHostPlayer.cpp \
	hoxHttpConnection.cpp \
	hoxHttpPlayer.cpp \
	hoxLocalPlayer.cpp \
	hoxMyPlayer.cpp \
	hoxNaiveReferee.cpp \
	hoxNetworkAPI.cpp \
	hoxPiece.cpp \
	hoxPlayer.cpp \
	hoxPlayerEvent.cpp \
	hoxPlayerMgr.cpp \
	hoxPosition.cpp \
	hoxReferee.cpp \
	hoxRemoteConnection.cpp \
	hoxRemotePlayer.cpp \
	hoxServer.cpp \
	hoxSocketConnection.cpp \
	hoxSocketServer.cpp \
	hoxTable.cpp \
	hoxTableMgr.cpp \
	hoxTablesDialog.cpp \
	hoxThreadConnection.cpp \
	hoxUtility.cpp \
	MyApp.cpp \
	MyChild.cpp \
	MyFrame.cpp

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(WX_CXXFLAGS) -c  -o $@ $<

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -rf $(PROGRAM) $(OBJECTS)

