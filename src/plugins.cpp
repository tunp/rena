#include "plugins.h"

#include <QPluginLoader>
#include <QDebug>
#include <QStringList>

#include "trackrecorder.h"

Plugins::Plugins(QObject *parent) : QObject(parent) {
	loadPlugins();
}

void Plugins::loadPlugins() {
    QPluginLoader loader("/usr/lib/rena/libUploadRunKeeper.so");
    UploadInterface *ui = qobject_cast<UploadInterface *>(loader.instance());
    if (ui) {
		uis.push_back(ui);
		qDebug() << "UploadInterface loaded";
	} else {
		qDebug() << "UploadInterface load failed";
	}
	
	QStringList track_infos;
	track_infos << "/usr/lib/rena/libTrackInfoBTLEBike.so";
	//track_infos	<< "/usr/lib/rena/libTrackInfoVirtual.so";
	for (int i = 0; i < track_infos.size(); i++) {
		QPluginLoader loader2(track_infos[i]);
		TrackInfoInterface *tii = qobject_cast<TrackInfoInterface *>(loader2.instance());
		if (tii) {
			tiis.push_back(tii);
			qDebug() << "TrackInfoInterface loaded" << track_infos[i];
			connect(tii->getObject(), SIGNAL(infoAvailable(TrackPoint)), SIGNAL(infoAvailable(TrackPoint)));
		} else {
			qDebug() << "TrackInfoInterface load failed" << track_infos[i];
		}
	}
}

void Plugins::uploadTrack(QString name) {
	foreach (UploadInterface *ui, uis) {
		ui->uploadTrack(name);
	}
}

QVariantList Plugins::getNames() {
	QVariantList list;
	foreach (UploadInterface *ui, uis) {
		QVariantMap new_data;
		new_data.insert("name", ui->getName());
		list.push_back(new_data);
	}
	return list;
}

void Plugins::openSettings(QString name) {
	foreach (UploadInterface *ui, uis) {
		if (ui->getName() == name) {
			ui->showSettings();
			qDebug() << "show settings";
		}
	}
}

void Plugins::changeTrackingStatus() {
	QObject *trackrecorder_obj = parent()->findChild<QObject*>("recorder");
	if (trackrecorder_obj) {
		TrackRecorder *trackrecorder = qobject_cast<TrackRecorder *>(trackrecorder_obj);
		if (trackrecorder) {
			foreach (TrackInfoInterface *tii, tiis) {
				tii->setTracking(trackrecorder->isTracking());
			}
		} else {
			qDebug() << "didn't get trackrecorder";
		}
	} else {
		qDebug() << "didn't get trackrecorder_obj";
	}
}
