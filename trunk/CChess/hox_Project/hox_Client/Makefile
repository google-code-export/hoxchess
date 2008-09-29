# Your project's name.
PROGRAM = hoxchess

# Common flags
CXX         = g++
WX_CXXFLAGS = `wx-config --cxxflags`
WX_LDLIBS   = `wx-config --libs`
LDFLAGS     = -pthread
DEBUGFLAGS  = -g

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
	engine.cpp \
	engine_data.cpp \
	generator.cpp \
	hash.cpp \
	history_data.cpp \
	move.cpp \
	search.cpp \
	xq.cpp \
	xq_data.cpp

# Define our sources and object files
SOURCES := \
	$(TSITO_SRC) \
	$(FOLIUM_SRC) \
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
	hoxAIPlayer.cpp \
	hoxTSITOPlayer.cpp \
	hoxFoliumPlayer.cpp \
	MyApp.cpp \
	MyChild.cpp \
	MyFrame.cpp

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(WX_CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: $(PROGRAM)

prepare:
	cp -v tsito/* .
	cp -v folium/* .

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -vrf $(PROGRAM) $(OBJECTS) $(TSITO_SRC) $(FOLIUM_SRC)
