#include <QObject>

class TrackInfoInterface {
public:
	virtual ~TrackInfoInterface() {}
	
	virtual void setTracking(bool tracking) = 0;
	virtual QObject* getObject() = 0;
};

Q_DECLARE_INTERFACE(TrackInfoInterface, "org.rena.TrackInfoInterface")
