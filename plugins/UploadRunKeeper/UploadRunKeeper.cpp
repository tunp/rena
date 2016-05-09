#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QList>

#include <QUrl>

#include <QGeoCoordinate>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QTimer>

#include "UploadRunKeeper.h"
#include "../../src/trackloader.h"

UploadRunKeeper::UploadRunKeeper() {
	settings = new QSettings("Simom", "rena-uploadrunkeeper");
	nam = new QNetworkAccessManager(this);
	QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedNetwork(QNetworkReply*)));
	initialized = false;
	QMetaObject::invokeMethod(this, "initRunKeeper");
}

UploadRunKeeper::~UploadRunKeeper() {
	delete settings;
}

void UploadRunKeeper::finishedNetwork(QNetworkReply* reply) {
	if (reply->url().toString() == "http://api.runkeeper.com/user") {
		if (reply->error() == QNetworkReply::NoError) {
			QByteArray reply_data = reply->readAll();
			qDebug() << QString(reply_data);
			QJsonDocument doc = QJsonDocument::fromJson(reply_data);
			if (!doc.isNull()) {
				QJsonObject obj = doc.object();
				fitness_activities = obj.value("fitness_activities").toString();
				qDebug() << "init success" << fitness_activities;
				initialized = true;
				uploading = true;
				QTimer::singleShot(1000, this, SLOT(uploadTrack()));
			} else {
				qDebug() << "init failed: could not get doc";
				QTimer::singleShot(60000, this, SLOT(initRunKeeper()));
			}
		} else {
			qDebug() << "init failed: network error" << reply->errorString();
			QTimer::singleShot(60000, this, SLOT(initRunKeeper()));
		}
	} else {
		if (reply->error() == QNetworkReply::NoError) {
			qDebug() << "upload success";
			QString name;
			getFirstTrackFromQueue(name);
			delTrackFromQueue(name);
			QTimer::singleShot(1000, this, SLOT(uploadTrack()));
		} else {
			qDebug() << "upload failed" << reply->errorString();
			qDebug() << reply->readAll();
			QTimer::singleShot(60000, this, SLOT(uploadTrack()));
		}
	}
}

void UploadRunKeeper::initRunKeeper() {
	QString auth;
	auth = settings->value("access_token").toString();
	QUrl url("http://api.runkeeper.com/user");
	QNetworkRequest request(url);
	request.setRawHeader("Authorization", QString("Bearer " + auth).toUtf8());
	request.setRawHeader("Connection", "Close");
	nam->get(request);
}

void UploadRunKeeper::showSettings() {
	qDebug() << "show settings";
}

void UploadRunKeeper::uploadTrack(QString filename) {
	addTrackToQueue(filename);
	if (initialized && !uploading) {
		uploading = true;
		QMetaObject::invokeMethod(this, "uploadTrack");
	}
}

void UploadRunKeeper::uploadTrack() {
	QString filename;
	if (getFirstTrackFromQueue(filename)) {
		qDebug() << "got track from queue and now uploading" << filename;
		TrackLoader loader;
		loader.setFilename(filename);
		QDateTime start_time = loader.trackPointTimeAt(0);
		QJsonObject json;
		double start_distance = 0;
		bool has_start_distance = false;
		json["type"] = QString("Cycling");
		json["start_time"] = QLocale::c().toString(start_time, "ddd, d MMM yyyy HH:mm:ss");
		json["notes"] = loader.description();
		json["total_distance"] = loader.distance();
		json["duration"] = (int) loader.duration();
		
		QJsonArray path;
		QJsonArray distance;
		for (int i = 0; i < loader.trackPointCount(); i++) {
			TrackPoint tp = loader.trackPointAt2(i);
			QDateTime time = loader.trackPointTimeAt(i);
			QJsonObject point;
			QJsonObject distance_point;
			point["timestamp"] = (int) start_time.secsTo(time);
			distance_point["timestamp"] = (int) start_time.secsTo(time);
			if (tp.hasCoordinate()) {
				point["altitude"] = tp.getElevation();
				point["longitude"] = tp.getLongitude();
				point["latitude"] = tp.getLatitude();
				if (i == 0) {
					point["type"] = QString("start");
				} else if (i == loader.trackPointCount()-1) {
					point["type"] = QString("end");
				} else {
					point["type"] = QString("gps");
				}
				path.push_back(point);
			}
			if (tp.hasDistance()) {
				if (!has_start_distance) {
					start_distance = tp.getDistance();
					has_start_distance = 1;
				}
				distance_point["distance"] = tp.getDistance() - start_distance;
				distance.push_back(distance_point);
			}
		}
		
		if (path.size() > 1) {
			json["path"] = path;
		}
		if (distance.size()) {
			json["distance"] = distance;
		}
		
		QJsonDocument doc;
		doc.setObject(json);
		
		QString auth;
		auth = settings->value("access_token").toString();
		QUrl url("http://api.runkeeper.com" + fitness_activities);
		QNetworkRequest request(url);
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.com.runkeeper.NewFitnessActivity+json");
		request.setRawHeader("Authorization", QString("Bearer " + auth).toUtf8());
		request.setRawHeader("Connection", "Close");
		nam->post(request, doc.toJson());
		
		qDebug() << "upload track";
	} else {
		uploading = false;
	}
}

QString UploadRunKeeper::getName() {
	return "Upload Runkeeper";
}

void UploadRunKeeper::setTrackQueue(QList<QString> &trackqueue) {
	settings->beginWriteArray("trackqueue");
	for (int i = 0; i < trackqueue.size(); i++) {
		settings->setArrayIndex(i);
		settings->setValue("name", trackqueue.at(i));
	}
	settings->endArray();
}

QList<QString> UploadRunKeeper::getTrackQueue() {
	QList<QString> trackqueue;
	int count = settings->beginReadArray("trackqueue");
	for (int i = 0; i < count; i++) {
		settings->setArrayIndex(i);
		trackqueue.push_back(settings->value("name").toString());
	}
	settings->endArray();
	return trackqueue;
}

void UploadRunKeeper::addTrackToQueue(QString name) {
	QList<QString> trackqueue = getTrackQueue();
	trackqueue.push_back(name);
	setTrackQueue(trackqueue);
}

void UploadRunKeeper::delTrackFromQueue(QString name) {
	QList<QString> trackqueue = getTrackQueue();
	trackqueue.removeAll(name);
	setTrackQueue(trackqueue);
}

bool UploadRunKeeper::getFirstTrackFromQueue(QString &name) {
	int count = settings->beginReadArray("trackqueue");
	if (count > 0) {
		settings->setArrayIndex(0);
		name = settings->value("name").toString();
		settings->endArray();
		return true;
	}
	settings->endArray();
	return false;
}
