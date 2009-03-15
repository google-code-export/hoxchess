####################################################################
# The 'Makefile' of HOXChess.
#
# Reference: 
#     http://wiki.wxwidgets.org/Makefile
#
####################################################################

# The name of the App.
PROGRAM = hoxchess

# Common flags
CXX         = g++

# RELEASE build
WX_CXXFLAGS = $(shell /opt/wxGTK-2.9.0/bin/wx-config --cxxflags --debug=no)
WX_LDLIBS   = $(shell /opt/wxGTK-2.9.0/bin/wx-config --libs --debug=no)
DEBUGFLAGS  =

# DEBUG build
#WX_CXXFLAGS = $(shell /opt/wxGTK-2.9.0/bin/wx-config --cxxflags --debug=yes)
#WX_LDLIBS   = $(shell /opt/wxGTK-2.9.0/bin/wx-config --libs --debug=yes)
#DEBUGFLAGS  = -g

CXXFLAGS = $(WX_CXXFLAGS) -I../lib/boost_1_38_0 -I../lib/asio-1.4.1/include
LDFLAGS     = -pthread

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
	hoxSavedTable.cpp \
	hoxAIPluginMgr.cpp \
	hoxWelcomeUI.cpp \
	hoxLog.cpp \
	MyApp.cpp \
	MyChild.cpp \
	MyFrame.cpp

# Define our source and object files
SOURCES := \
	$(MAIN_SRC)

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: $(PROGRAM)

prepare:
	@echo No longer needed.

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(WX_LDLIBS)

clean:
	rm -vrf $(PROGRAM) $(OBJECTS)

############## END OF FILE ###############################################

