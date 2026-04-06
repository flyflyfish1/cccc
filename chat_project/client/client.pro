QT += core gui widgets network
CONFIG += c++11

TARGET = chat_client
TEMPLATE = app

INCLUDEPATH += ../shared

SOURCES += \
    main.cpp \
    loginwindow.cpp \
    registerwindow.cpp \
    mainwindow.cpp \
    chatwindow.cpp \
    networkclient.cpp \
    filehelper.cpp \
    ../shared/protocol.cpp

HEADERS += \
    loginwindow.h \
    registerwindow.h \
    mainwindow.h \
    chatwindow.h \
    networkclient.h \
    filehelper.h \
    ../shared/protocol.h
