#include <QObject>
#include <QtPlugin>

#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "../UploadInterface.h"

class UploadRunKeeper : public QObject, public UploadInterface {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.rena.UploadInterface")
	Q_INTERFACES(UploadInterface)
public slots:
    void finishedNetwork(QNetworkReply*);
    void initRunKeeper();
    void uploadTrack();
private:
	QSettings *settings;
	QNetworkAccessManager* nam;
	bool uploading;
	bool initialized;
	QString fitness_activities;
public:
	UploadRunKeeper();
	~UploadRunKeeper();
	void showSettings();
	void uploadTrack(QString filename);
	QString getName();
	void setTrackQueue(QList<QString> &trackqueue);
	QList<QString> getTrackQueue();
	void addTrackToQueue(QString name);
	void delTrackFromQueue(QString name);
	bool getFirstTrackFromQueue(QString &name);
};
