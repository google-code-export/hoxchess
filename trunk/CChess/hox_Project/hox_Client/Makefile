# Your project's name.
PROGRAM = hoxchess

# Common flags
CXX         = g++
#WX_CXXFLAGS = `wx-config --cxxflags`
WX_CXXFLAGS = -I/usr/lib/wx/include/gtk2-ansi-release-2.8 \
              -I/usr/include/wx-2.8 \
              -D_FILE_OFFSET_BITS=64 \
              -D_LARGE_FILES -D__WXGTK__ \
              -pthread
#WX_LDLIBS   = `wx-config --libs`
WX_LDLIBS   = -L/usr/lib -pthread \
              -lwx_gtk2_richtext-2.8 \
              -lwx_gtk2_aui-2.8 \
              -lwx_gtk2_xrc-2.8 \
              -lwx_gtk2_qa-2.8 \
              -lwx_gtk2_html-2.8 \
              -lwx_gtk2_adv-2.8 \
              -lwx_gtk2_core-2.8 \
              -lwx_base_xml-2.8 \
              -lwx_base_net-2.8 \
              -lwx_base-2.8 
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
	hoxAIPlayer.cpp \
	hoxTSITOPlayer.cpp \
	hoxFoliumPlayer.cpp \
	hoxXQWLightPlayer.cpp \
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
	$(CXX) $(WX_CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: $(PROGRAM)

prepare:
	cp -v ../xqwlight/*.h ../xqwlight/*.cpp .
	cp -v ../tsito/* .
	cp -v ../folium/* .

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -vrf $(PROGRAM) $(OBJECTS) $(XQWLight_SRC) $(TSITO_SRC) $(FOLIUM_SRC)
