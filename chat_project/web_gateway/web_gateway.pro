QT += core network websockets
CONFIG += console c++11
CONFIG -= app_bundle

TARGET = chat_web_gateway
TEMPLATE = app

INCLUDEPATH += ../shared

SOURCES += \
    main.cpp \
    bridgesession.cpp \
    ../shared/protocol.cpp

HEADERS += \
    bridgesession.h \
    ../shared/protocol.h
