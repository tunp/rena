#include <QString>

class UploadInterface {
public:
	virtual ~UploadInterface() {}
	
	virtual void showSettings() = 0;
	virtual void uploadTrack(QString filename) = 0;
	virtual QString getName() = 0;
};

Q_DECLARE_INTERFACE(UploadInterface, "org.rena.UploadInterface")
