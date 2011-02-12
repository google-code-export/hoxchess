# -------------------------------------------------
# Project created by QtCreator 2010-04-05T08:07:45
# -------------------------------------------------
TARGET = QtXiangqi
TEMPLATE = app
INCLUDEPATH += /opt/local/include
INCLUDEPATH += ./lib/asio-1.4.1/include
LIBS += /opt/local/lib/libboost_thread-mt.a
SOURCES += main.cpp \
    mainwindow.cpp \
    piece.cpp \
    board.cpp \
    Referee/Referee.cpp \
    Referee/XQWLight_Referee.cpp \
    AI/AI_XQWLight.cpp \
    AI/XQWLight.cpp \
    tableui.cpp \
    network/hoxAsyncSocket.cpp \
    network/hoxSocketConnection.cpp \
    logindialog.cpp \
    message/hoxMessage.cpp \
    common/hoxUtil.cpp \
    tablelistui.cpp \
    aiboardcontroller.cpp \
    networkboardcontroller.cpp
HEADERS += mainwindow.h \
    piece.h \
    board.h \
    enums.h \
    types.h \
    Referee/Referee.h \
    AI/AIEngine.h \
    AI/AI_XQWLight.h \
    network/hoxAsyncSocket.h \
    network/hoxSocketConnection.h \
    logindialog.h \
    tableui.h \
    message/hoxMessage.h \
    common/hoxUtil.h \
    tablelistui.h \
    aiboardcontroller.h \
    networkboardcontroller.h \
    shared.h
FORMS += mainwindow.ui \
    tableui.ui \
    logindialog.ui \
    tablelistui.ui
RESOURCES += board.qrc
OTHER_FILES += \ 
    README.txt
