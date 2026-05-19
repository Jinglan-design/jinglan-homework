QT += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    minefield.cpp \
    cellbutton.cpp

HEADERS += \
    mainwindow.h \
    minefield.h \
    cellbutton.h

DEFINES += QT_DEPRECATED_WARNINGS

RESOURCES += \
    resources.qrc