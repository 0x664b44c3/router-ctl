INCLUDEPATH +=$$PWD

QT += core network serialport
CONFIG += c++11

INCLUDEPATH+=$$PWD
SOURCES +=\
         $$PWD/abstractbusdriver.cpp \
         $$PWD/bmd_videohub_driver.cpp \
         $$PWD/busmanager.cpp \
         $$PWD/gvgbusdrivers.cpp \
         $$PWD/iostreambusbase.cpp \
         $$PWD/leitchbusdriver.cpp \
         $$PWD/matrix.cpp \
         $$PWD/matrixmanager.cpp \
         $$PWD/quartzprotocol.cpp \
         $$PWD/routermanager.cpp \
         $$PWD/termctl.cpp \
         $$PWD/vikinx_protocol.cpp

HEADERS += \
         $$PWD/abstractbusdriver.h \
         $$PWD/bmd_videohub_driver.h \
         $$PWD/busmanager.h \
         $$PWD/gvgbusdrivers.h \
         $$PWD/iostreambusbase.h \
         $$PWD/json-utils.h \
         $$PWD/leitchbusdriver.h \
         $$PWD/matrix.h \
         $$PWD/matrixmanager.h \
         $$PWD/quartzprotocol.h \
         $$PWD/routermanager.h \
         $$PWD/termctl.h \
         $$PWD/vikinx_protocol.h
