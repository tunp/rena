#include <QTimer>

#include "TrackInfoVirtual.h"

TrackInfoVirtual::TrackInfoVirtual() {
	running = 0;
	counter = 0;
}

TrackInfoVirtual::~TrackInfoVirtual() {
	qDebug() << "TrackInfoVirtual destructor";
}

void TrackInfoVirtual::setTracking(bool tracking) {
	if (tracking) {
		running = 1;
		this->moveToThread(&workerThread);
		QMetaObject::invokeMethod(this, "worker");
		workerThread.start();
	} else {
		running = 0;
		workerThread.quit();
	}
}

void TrackInfoVirtual::worker() {
	if (!running) {
		return;
	}
	TrackPoint p;
	p.setDistance(counter);
	p.setCadence(counter*2);
	p.setTime(QDateTime::currentDateTimeUtc());
	emit infoAvailable(p);
	counter++;
	QTimer::singleShot(1000, this, SLOT(worker()));
}
