QT += core network sql
CONFIG += console c++11
CONFIG -= app_bundle

TARGET = chat_server
TEMPLATE = app

INCLUDEPATH += ../shared

SOURCES += \
    main.cpp \
    chatserver.cpp \
    clienthandler.cpp \
    dbmanager.cpp \
    logger.cpp \
    ../shared/protocol.cpp

HEADERS += \
    chatserver.h \
    clienthandler.h \
    dbmanager.h \
    logger.h \
    ../shared/protocol.h
