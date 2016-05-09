#include <QObject>
#include <QtPlugin>
#include <QThread>

#include "../TrackInfoInterface.h"
#include "../../src/TrackPoint.h"

class TrackInfoVirtual : public QObject, public TrackInfoInterface {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.rena.TrackInfoInterface")
	Q_INTERFACES(TrackInfoInterface)
public slots:
    void worker();
signals:
	void infoAvailable(TrackPoint info);
private:
	bool running;
	int counter;
	QThread workerThread;
public:
	TrackInfoVirtual();
	~TrackInfoVirtual();
	void setTracking(bool tracking);
	QObject* getObject() {return this;}
};
