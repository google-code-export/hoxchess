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
	Engine.cpp \
	Evaluator.cpp \
	Lawyer.cpp \
	Move.cpp \
	OpeningBook.cpp \
	Options.cpp \
	Timer.cpp \
	Transposition.cpp

# Define our sources and object files
SOURCES := \
	$(TSITO_SRC) \
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
	MyApp.cpp \
	MyChild.cpp \
	MyFrame.cpp

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(WX_CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: prepare $(PROGRAM)

prepare:
	cp tsito/* .

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -rf $(PROGRAM) $(OBJECTS)
