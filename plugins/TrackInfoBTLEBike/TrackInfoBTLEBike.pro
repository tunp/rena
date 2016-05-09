TEMPLATE = lib
CONFIG += plugin
QT += positioning location
LIBS += -lbluetooth

SOURCES += TrackInfoBTLEBike.cpp
HEADERS += TrackInfoBTLEBike.h \
			SpdData.h \
			../../src/TrackPoint.h

uploads.path = /usr/lib/rena
uploads.files = *.so
uploads.extra = mkdir -p ${INSTALL_ROOT}/usr/lib/rena
INSTALLS += uploads
