#include <QDebug>
#include <QTimer>

#include <cerrno>

#include <sys/socket.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include "TrackInfoBTLEBike.h"
#include "SpdData.h"

TrackInfoBTLEBike::TrackInfoBTLEBike() {
	settings = new QSettings("Simom", "rena-trackinfobtlebike");
	thread_exit = 1;
}

TrackInfoBTLEBike::~TrackInfoBTLEBike() {
	qDebug() << "TrackInfoBTLEBike destructor";
	delete settings;
	closeThread();
}

void TrackInfoBTLEBike::closeThread() {
	thread_exit = 1;
	shutdown(sock, SHUT_WR);
	workerThread.quit();
	workerThread.wait();
	close(sock);
}

void TrackInfoBTLEBike::setTracking(bool tracking) {
	if (tracking) {
		this->moveToThread(&workerThread);
		thread_exit = 0;
		QMetaObject::invokeMethod(this, "connect");
		workerThread.start();
	} else {
		closeThread();
	}
}

void TrackInfoBTLEBike::connect() {
	struct sockaddr_l2 addr = {};
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	((char *)&addr)[10] = 4;
	int success = 1;
	
	mac = settings->value("mac").toString();
	circuit = settings->value("circuit").toInt();
	if (!mac.size() || !circuit) {
		success = 0;
	}
	
	sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock == -1) {
		qDebug() << "couldnt create socket " << strerror(errno);
		success = 0;
	}
	if (success && bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		qDebug() << "bind failed " << strerror(errno);
		close(sock);
		success = 0;
	}
	((char *)&addr)[12] = 1;
	qDebug() << "connecting to: " << mac;
	str2ba(mac.toStdString().c_str(), &addr.l2_bdaddr);
	if (success && ::connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		qDebug() << "connect failed " << strerror(errno);
		close(sock);
		success = 0;
	}
	
	if (success && write(sock, "\x12\x13\x00\x01\x00", 5) != 5) {
		qDebug() << "failed to write " << strerror(errno);
		close(sock);
		success = 0;
	}

	if (success) {
		qDebug() << "connected, going to read loop";
		read();
	}
	
	QTimer::singleShot(5000, this, SLOT(connect()));
}

void TrackInfoBTLEBike::read() {
	int red;
	char buf[512];
	SpdData *last_spd_data = 0;
	while (!thread_exit) {
		red = ::read(sock, buf, sizeof(buf));
		if (red < 0) {
			qDebug() << "reading socket failed " << strerror(errno);
			break;
		} else if (red == 0) {
			qDebug() << "socket closed " << strerror(errno);
			break;
		} else {
			SpdData *spd_data = new SpdData;
			memset(spd_data, 0, sizeof(SpdData));
			if (red == 10) {
				memcpy(spd_data, buf, 10);
			} else if (red == 8) {
				memcpy(spd_data, buf, 4);
				memcpy(spd_data+10, buf+4, 4);
			} else if (red == 14) {
				memcpy(spd_data, buf, 14);
			}
			if (spd_data->mode) {
				TrackPoint tp;
				if (spd_data->mode & 1 && spd_data->distance_cnt > 0) {
					tp.setDistance((double) spd_data->distance_cnt * circuit / 1000);
				}
				if (last_spd_data && last_spd_data->mode & 1 && spd_data->mode & 1 && last_spd_data->distance_cnt > 0 && spd_data->distance_cnt > 0) {
					unsigned short d_distance_cnt = spd_data->distance_cnt - last_spd_data->distance_cnt;
					int d_distance_u = (int) spd_data->distance_u - last_spd_data->distance_u;
					if (d_distance_u < 0) {
						d_distance_u += 0xffff;
					}
					if (d_distance_cnt > 0) {
						double speed = (double) circuit / ((double) d_distance_u / d_distance_cnt);
						tp.setGroundSpeed(speed);
					}
				}
				if (last_spd_data && last_spd_data->mode & 2 && spd_data->mode & 2 && last_spd_data->cadence_cnt > 0 && spd_data->cadence_cnt > 0) {
					unsigned short d_cadence_cnt = spd_data->cadence_cnt - last_spd_data->cadence_cnt;
					int d_cadence_u = (int) spd_data->cadence_u - last_spd_data->cadence_u;
					if (d_cadence_u < 0) {
						d_cadence_u += 0xffff;
					}
					if (d_cadence_cnt > 0) {
						double cadence = ((double) d_cadence_u / d_cadence_cnt);
						tp.setCadence(cadence);
					}
				}
				tp.setTime(QDateTime::currentDateTimeUtc());
				qDebug() << "info available" << tp.getDistance() << tp.getCadence();
				emit infoAvailable(tp);
				delete last_spd_data;
				last_spd_data = spd_data;
			}
		}
	}
}
