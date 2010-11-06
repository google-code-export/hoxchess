# -------------------------------------------------
# Project created by QtCreator 2010-04-05T08:07:45
# -------------------------------------------------
TARGET = QtXiangqi
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    piece.cpp \
    board.cpp \
    Referee/Referee.cpp \
    Referee/XQWLight_Referee.cpp \
    AI/AI_XQWLight.cpp \
    AI/XQWLight.cpp \
    tableui.cpp
HEADERS += mainwindow.h \
    piece.h \
    board.h \
    enums.h \
    types.h \
    Referee/Referee.h \
    AI/AIEngine.h \
    AI/AI_XQWLight.h \
    tableui.h
FORMS += mainwindow.ui \
    tableui.ui
RESOURCES += board.qrc
OTHER_FILES += \ 
    README.txt
