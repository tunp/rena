#ifndef PLUGINS_H
#define PLUGINS_H

#include <QObject>
#include <QVariant>
#include <QList>

#include "../plugins/UploadInterface.h"
#include "../plugins/TrackInfoInterface.h"
#include "TrackPoint.h"

class Plugins : public QObject
{
    Q_OBJECT
public:
    explicit Plugins(QObject *parent = 0);
	void loadPlugins();
	void uploadTrack(QString name);
	Q_INVOKABLE QVariantList getNames();
	Q_INVOKABLE void openSettings(QString name);
signals:
	void infoAvailable(TrackPoint info);
public slots:
	void changeTrackingStatus();
private:
    QList<UploadInterface *> uis;
    QList<TrackInfoInterface *> tiis;
};

#endif
