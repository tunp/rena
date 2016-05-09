TEMPLATE = lib
CONFIG += plugin
QT += positioning location

SOURCES += UploadRunKeeper.cpp \
	../../src/trackloader.cpp
HEADERS += UploadRunKeeper.h \
	../../src/trackloader.h
OTHERS += qml/UploadRunKeeper.qml

uploads.path = /usr/lib/rena
uploads.files = *.so
uploads.extra = mkdir -p ${INSTALL_ROOT}/usr/lib/rena
upload_qmls.path = /usr/share/harbour-rena/qml/plugins
upload_qmls.files = qml/*.qml
upload_qmls.extra = mkdir -p ${INSTALL_ROOT}/usr/share/harbour-rena/qml/plugins
INSTALLS += uploads upload_qmls
