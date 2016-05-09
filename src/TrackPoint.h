#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QObject>
#include <QDateTime>
#include <QGeoPositionInfoSource>
#include <QDebug>

class TrackPoint {
public:
	qreal latitude;
	qreal longitude;
	QDateTime time;
	qreal elevation;
	qreal direction;
	qreal groundSpeed;
	qreal verticalSpeed;
	qreal magneticVariation;
	qreal horizontalAccuracy;
	qreal verticalAccuracy;
	qreal distance;
	qreal cadence;
	
	bool has_coordinate;
	bool has_time;
	bool has_elevation;
	bool has_direction;
	bool has_ground_speed;
	bool has_vertical_speed;
	bool has_magnetic_variation;
	bool has_horizontal_accuracy;
	bool has_vertical_accuracy;
	bool has_distance;
	bool has_cadence;
	
	TrackPoint() {
		reset();
	}
	TrackPoint(QGeoPositionInfo info) {
		reset();
		setQGeoPositionInfo(info);
	}
	
	void reset() {
		 has_coordinate = 0;
		 has_time = 0;
		 has_elevation = 0;
		 has_direction = 0;
		 has_ground_speed = 0;
		 has_vertical_speed = 0;
		 has_magnetic_variation = 0;
		 has_horizontal_accuracy = 0;
		 has_vertical_accuracy = 0;
		 has_distance = 0;
		 has_cadence = 0;
	}
	
	void setQGeoPositionInfo(QGeoPositionInfo info) {
        setLatitude(info.coordinate().latitude());
        setLongitude(info.coordinate().longitude());
        setTime(info.timestamp());
        if(info.coordinate().type() == QGeoCoordinate::Coordinate3D) {
            setElevation(info.coordinate().altitude());
        }
        if(info.hasAttribute(QGeoPositionInfo::Direction)) {
            setDirection(info.attribute(QGeoPositionInfo::Direction));
        }
        if(info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
            setGroundSpeed(info.attribute(QGeoPositionInfo::GroundSpeed));
        }
        if(info.hasAttribute(QGeoPositionInfo::VerticalSpeed)) {
            setVerticalSpeed(info.attribute(QGeoPositionInfo::VerticalSpeed));
        }
        if(info.hasAttribute(QGeoPositionInfo::MagneticVariation)) {
            setMagneticVariation(info.attribute(QGeoPositionInfo::MagneticVariation));
        }
        if(info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
            setHorizontalAccuracy(info.attribute(QGeoPositionInfo::HorizontalAccuracy));
        }
        if(info.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
            setVerticalAccuracy(info.attribute(QGeoPositionInfo::VerticalAccuracy));
        }
	}
	
	void combine(TrackPoint &other, bool overwrite) {
		if (other.hasCoordinate() && (!hasCoordinate() || overwrite)) {
			setLatitude(other.getLatitude());
			setLongitude(other.getLongitude());
		}
		if (other.hasTime() && (!hasTime() || overwrite)) {
			setTime(other.getTime());
		}
		if (other.hasElevation() && (!hasElevation() || overwrite)) {
			setElevation(other.getElevation());
		}
		if (other.hasDirection() && (!hasDirection() || overwrite)) {
			setDirection(other.getDirection());
		}
		if (other.hasGroundSpeed() && (!hasGroundSpeed() || overwrite)) {
			setGroundSpeed(other.getGroundSpeed());
		}
		if (other.hasVerticalSpeed() && (!hasVerticalSpeed() || overwrite)) {
			setVerticalSpeed(other.getVerticalSpeed());
		}
		if (other.hasMagneticVariation() && (!hasMagneticVariation() || overwrite)) {
			setMagneticVariation(other.getMagneticVariation());
		}
		if (other.hasHorizontalAccuracy() && (!hasHorizontalAccuracy() || overwrite)) {
			setHorizontalAccuracy(other.getHorizontalAccuracy());
		}
		if (other.hasVerticalAccuracy() && (!hasVerticalAccuracy() || overwrite)) {
			setVerticalAccuracy(other.getVerticalAccuracy());
		}
		if (other.hasDistance() && (!hasDistance() || overwrite)) {
			setDistance(other.getDistance());
		}
		if (other.hasCadence() && (!hasCadence() || overwrite)) {
			setCadence(other.getCadence());
		}
	}
	
	void setLatitude(qreal latitude) {this->latitude = latitude; if (latitude != 0 && latitude == latitude) {has_coordinate = 1;} }
	void setLongitude(qreal longitude) {this->longitude = longitude; if (longitude != 0 && latitude == latitude) {has_coordinate = 1;} }
	void setTime(QDateTime time) {this->time = time; has_time = 1;}
	void setElevation(qreal elevation) {this->elevation = elevation; has_elevation = 1;}
	void setDirection(qreal direction) {this->direction = direction; has_direction = 1;}
	void setGroundSpeed(qreal ground_speed) {this->groundSpeed = ground_speed; has_ground_speed = 1;}
	void setVerticalSpeed(qreal vertical_speed) {this->verticalSpeed = vertical_speed; has_vertical_speed = 1;}
	void setMagneticVariation(qreal magnetic_variation) {this->magneticVariation = magnetic_variation; has_magnetic_variation = 1;}
	void setHorizontalAccuracy(qreal horizontal_accuracy) {this->horizontalAccuracy = horizontal_accuracy; has_horizontal_accuracy = 1;}
	void setVerticalAccuracy(qreal vertical_accuracy) {this->verticalAccuracy = vertical_accuracy; has_vertical_accuracy = 1;}
	void setDistance(qreal distance) {this->distance = distance; has_distance = 1;}
	void setCadence(qreal cadence) {this->cadence = cadence; has_cadence = 1;}
	
	bool hasCoordinate() const {return has_coordinate;}
	bool hasTime() const {return has_time;}
	bool hasElevation() const {return has_elevation;}
	bool hasDirection() const {return has_direction;}
	bool hasGroundSpeed() const {return has_ground_speed;}
	bool hasVerticalSpeed() const {return has_vertical_speed;}
	bool hasMagneticVariation() const {return has_magnetic_variation;}
	bool hasHorizontalAccuracy() const {return has_horizontal_accuracy;}
	bool hasVerticalAccuracy() const {return has_vertical_accuracy;}
	bool hasDistance() const {return has_distance;}
	bool hasCadence() const {return has_cadence;}
	
	qreal getLatitude() const {return latitude;}
	qreal getLongitude() const {return longitude;}
	QDateTime getTime() const {return time;}
	qreal getElevation() const {return elevation;}
	qreal getDirection() const {return direction;}
	qreal getGroundSpeed() const {return groundSpeed;}
	qreal getVerticalSpeed() const {return verticalSpeed;}
	qreal getMagneticVariation() const {return magneticVariation;}
	qreal getHorizontalAccuracy() const {return horizontalAccuracy;}
	qreal getVerticalAccuracy() const {return verticalAccuracy;}
	qreal getDistance() const {return distance;}
	qreal getCadence() const {return cadence;}
};

#endif
