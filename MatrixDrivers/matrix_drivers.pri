QT += core network serialport
CONFIG += c++11

INCLUDEPATH+=$$PWD
SOURCES +=\
#	 $$PWD/videomatrixemu.cpp \#
#	 $$PWD/abstractvideomatrix.cpp \
#	 $$PWD/matrixjsonexporter.cpp \
#	 $$PWD/networkvikinxmatrix.cpp \
#	 $$PWD/videohubserver.cpp \
	 $$PWD/abstractbusdriver.cpp \
	 $$PWD/leitchbusdriver.cpp \
	 $$PWD/quartzrouter.cpp

HEADERS += \
#	 $$PWD/videomatrixemu.h \
#	 $$PWD/abstractvideomatrix.h \
#	 $$PWD/matrixjsonexporter.h \
#	 $$PWD/networkvikinxmatrix.h \
#	 $$PWD/nevionprotocol.h \
#	 $$PWD/videohubserver.h \
	 $$PWD/abstractbusdriver.h \
	 $$PWD/leitchbusdriver.h \
	 $$PWD/quartzrouter.h
