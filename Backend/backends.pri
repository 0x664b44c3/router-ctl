INCLUDEPATH +=$$PWD

QT += core network serialport
CONFIG += c++11

INCLUDEPATH+=$$PWD
SOURCES +=\
         $$PWD/abstractbusdriver.cpp \
         $$PWD/busmanager.cpp \
         $$PWD/iostreambusbase.cpp \
         $$PWD/leitchbusdriver.cpp \
         $$PWD/matrix.cpp \
         $$PWD/quartzprotocol.cpp \
         $$PWD/routermanager.cpp

HEADERS += \
         $$PWD/abstractbusdriver.h \
         $$PWD/busmanager.h \
         $$PWD/iostreambusbase.h \
         $$PWD/leitchbusdriver.h \
         $$PWD/matrix.h \
         $$PWD/quartzprotocol.h \
         $$PWD/routermanager.h
