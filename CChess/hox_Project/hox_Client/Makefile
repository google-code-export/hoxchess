# Your project's name.
PROGRAM = hoxchess

# Common flags
CXX         = g++
WX_CXXFLAGS = `/opt/wxGTK-2.9.0/bin/wx-config --cxxflags`
WX_LDLIBS   = `/opt/wxGTK-2.9.0/bin/wx-config --libs`
CXXFLAGS = $(WX_CXXFLAGS) \
	   -I../lib/boost_1_36_0
LDFLAGS     = -pthread
DEBUGFLAGS  = -g

# XQWLight AI Engine source
XQWLight_SRC := \
 	XQWLight.cpp

# TSITO AI Engine source
TSITO_SRC := \
	Board.cpp \
	tsiEngine.cpp \
	Evaluator.cpp \
	Lawyer.cpp \
	Move.cpp \
	OpeningBook.cpp \
	Options.cpp \
	Timer.cpp \
	Transposition.cpp

# FOLIUM AI Engine source
FOLIUM_SRC := \
	bitmap_data.cpp \
	folEngine.cpp \
	engine_data.cpp \
	generator.cpp \
	hash.cpp \
	history_data.cpp \
	move.cpp \
	folSearch.cpp \
	xq.cpp \
	xq_data.cpp \
	folHOXEngine.cpp

# The main source
MAIN_SRC := \
	hoxBoard.cpp \
	hoxChesscapeConnection.cpp \
	hoxChesscapePlayer.cpp \
	hoxConnection.cpp \
	hoxCoreBoard.cpp \
	hoxLocalPlayer.cpp \
	hoxLoginDialog.cpp \
	hoxMyPlayer.cpp \
	hoxNetworkAPI.cpp \
	hoxPiece.cpp \
	hoxPlayer.cpp \
	hoxPlayerMgr.cpp \
	hoxReferee.cpp \
	hoxSocketConnection.cpp \
	hoxTable.cpp \
	hoxTableMgr.cpp \
	hoxTablesDialog.cpp \
	hoxUtil.cpp \
	hoxSite.cpp \
	hoxOptionDialog.cpp \
	hoxTypes.cpp \
	hoxPlayersUI.cpp \
	hoxSitesUI.cpp \
	hoxOptionsUI.cpp \
	hoxAIPlayer.cpp \
	hoxTSITOPlayer.cpp \
	hoxFoliumPlayer.cpp \
	hoxXQWLightPlayer.cpp \
	hoxSavedTable.cpp \
	hoxLog.cpp \
	MyApp.cpp \
	MyChild.cpp \
	MyFrame.cpp

# Define our sources and object files
SOURCES := \
	$(XQWLight_SRC) \
	$(TSITO_SRC) \
	$(FOLIUM_SRC) \
	$(MAIN_SRC)

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: $(PROGRAM)

prepare:
	cp -v ../xqwlight/*.h ../xqwlight/*.cpp .
	cp -v ../tsito/* .
	cp -v ../folium/* .

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -vrf $(PROGRAM) $(OBJECTS) $(XQWLight_SRC) $(TSITO_SRC) $(FOLIUM_SRC)
