#include <QObject>
#include <QtPlugin>
#include <QThread>
#include <QSettings>
#include <QString>

#include "../TrackInfoInterface.h"
#include "../../src/TrackPoint.h"

class TrackInfoBTLEBike : public QObject, public TrackInfoInterface {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.rena.TrackInfoInterface")
	Q_INTERFACES(TrackInfoInterface)
public slots:
    void connect();
    void read();
signals:
	void infoAvailable(TrackPoint info);
private:
	int sock;
	QString mac;
	short circuit;
	QThread workerThread;
	bool thread_exit;
	QSettings *settings;
public:
	TrackInfoBTLEBike();
	~TrackInfoBTLEBike();
	void closeThread();
	void setTracking(bool tracking);
	QObject* getObject() {return this;}
};
