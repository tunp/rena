TEMPLATE = lib
CONFIG += plugin
QT += positioning location

SOURCES += TrackInfoVirtual.cpp
HEADERS += TrackInfoVirtual.h \
			../../src/TrackPoint.h

uploads.path = /usr/lib/rena
uploads.files = *.so
uploads.extra = mkdir -p ${INSTALL_ROOT}/usr/lib/rena
INSTALLS += uploads
