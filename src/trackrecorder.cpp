/*
    Copyright 2014 Simo Mattila
    simo.h.mattila@gmail.com

    This file is part of Rena.

    Rena is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rena is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rena.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QStandardPaths>
#include <QDir>
#include <QSaveFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <qmath.h>
#include <iterator>
#include "trackrecorder.h"

TrackRecorder::TrackRecorder(QObject *parent) :
    QObject(parent)
{
    qDebug()<<"TrackRecorder constructor";
    m_distance = 0.0;
    m_accuracy = -1;
    m_tracking = false;
    m_isEmpty = true;
    m_applicationActive = true;
    m_autoSavePosition = 0;
    last_position_time = 0;
    last_distance_time = 0;

    // Load autosaved track if left from previous session
    loadAutoSave();

    // Setup periodic autosave
    m_autoSaveTimer.setInterval(60000);
    connect(&m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSave()));
    m_autoSaveTimer.start();

    m_posSrc = QGeoPositionInfoSource::createDefaultSource(0);
    if (m_posSrc) {
        m_posSrc->setUpdateInterval(1000);
        connect(m_posSrc, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(positionUpdated(QGeoPositionInfo)));
        connect(m_posSrc, SIGNAL(error(QGeoPositionInfoSource::Error)),
                this, SLOT(positioningError(QGeoPositionInfoSource::Error)));
        // Position updates are started/stopped in setIsTracking(...)
    } else {
        qDebug()<<"Failed initializing PositionInfoSource!";
    }
    QTimer::singleShot(1000, this, SLOT(connectPlugins()));
    qDebug()<<"tr end";
}

TrackRecorder::~TrackRecorder() {
    qDebug()<<"TrackRecorder destructor";
    autoSave();
}

void TrackRecorder::connectPlugins() {
	QObject *plugins_obj = parent()->findChild<QObject*>("plugins");
	if (plugins_obj) {
		qDebug() << "got plugins_obj";
		plugins = qobject_cast<Plugins *>(plugins_obj);
		if (plugins) {
			qDebug() << "got plugins for setting signal";
			connect(plugins, SIGNAL(infoAvailable(TrackPoint)), this, SLOT(positionUpdated(TrackPoint)));
			connect(this, SIGNAL(isTrackingChanged()), plugins, SLOT(changeTrackingStatus()));
		} else {
			qDebug() << "didn't get plugins for setting signal";
			QTimer::singleShot(1000, this, SLOT(connectPlugins()));
		}
	} else {
		qDebug() << "didn't get plugins_obj";
		QTimer::singleShot(1000, this, SLOT(connectPlugins()));
	}
}

void TrackRecorder::positionUpdated(const QGeoPositionInfo &newPos) {
    if(newPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        m_accuracy = newPos.attribute(QGeoPositionInfo::HorizontalAccuracy);
    } else {
        m_accuracy = -1;
    }
    emit accuracyChanged();

    m_currentPosition = newPos.coordinate();
    emit currentPositionChanged();

    if(newPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) &&
            (newPos.attribute(QGeoPositionInfo::HorizontalAccuracy) > 30.0)) {
        return;
    }

    if(m_tracking) {
		TrackPoint tp(newPos);
		if (tp.hasCoordinate()) {
			if (last_position_time != 0 && last_position_time < tp.getTime().toTime_t()) {
				TrackPoint *last_position_point = &m_points[last_position_time];
				QGeoCoordinate coord(last_position_point->getLatitude(), last_position_point->getLongitude());
				m_distance += coord.distanceTo(newPos.coordinate());
				last_distance_time = 0;
			}
			last_position_time = tp.getTime().toTime_t();
		}
		
		m_points[newPos.timestamp().toTime_t()].combine(tp, true);
        
        emit pointsChanged();
        emit timeChanged();
        if(m_isEmpty) {
            m_isEmpty = false;
            m_minLat = m_maxLat = newPos.coordinate().latitude();
            m_minLon = m_maxLon = newPos.coordinate().longitude();
            emit isEmptyChanged();
        }

        if(m_points.size() > 1) {
            // Next line triggers following compiler warning?
            // \usr\include\qt5\QtCore\qlist.h:452: warning: assuming signed overflow does not occur when assuming that (X - c) > X is always false [-Wstrict-overflow]
            emit distanceChanged();
            if(newPos.coordinate().latitude() < m_minLat) {
                m_minLat = newPos.coordinate().latitude();
            } else if(newPos.coordinate().latitude() > m_maxLat) {
                m_maxLat = newPos.coordinate().latitude();
            }
            if(newPos.coordinate().longitude() < m_minLon) {
                m_minLon = newPos.coordinate().longitude();
            } else if(newPos.coordinate().longitude() > m_maxLon) {
                m_maxLon = newPos.coordinate().longitude();
            }
        }
        emit newTrackPoint(newPos.coordinate());
    }
}

void TrackRecorder::positionUpdated(TrackPoint newPoint) {
	if (m_tracking) {
		qDebug() << "check1" << &newPoint << newPoint.getDistance();
		m_points[newPoint.getTime().toTime_t()].combine(newPoint, false);
		qDebug() << "check2" << &m_points[newPoint.getTime().toTime_t()] << m_points[newPoint.getTime().toTime_t()].getDistance();
		
        emit pointsChanged();
        emit timeChanged();
        if(m_isEmpty) {
            m_isEmpty = false;
            emit isEmptyChanged();
        }
        
        if (newPoint.hasDistance()) {
			qDebug() << "new track distance" << newPoint.getDistance();
			if ((last_position_time == 0 || last_position_time < newPoint.getTime().toTime_t() - 5) && last_distance_time != 0 && last_distance_time < newPoint.getTime().toTime_t()) {
				TrackPoint *last_distance_point = &m_points[last_distance_time];
				m_distance += newPoint.getDistance() - last_distance_point->getDistance();
				last_position_time = 0;
			}
			last_distance_time = newPoint.getTime().toTime_t();
		}
        emit distanceChanged();
	}
}

void TrackRecorder::positioningError(QGeoPositionInfoSource::Error error) {
    qDebug()<<"Positioning error:"<<error;
}

void TrackRecorder::exportGpx(QString name, QString desc) {
    qDebug()<<"Exporting track to gpx";
    if(m_points.size() < 1) {
        qDebug()<<"Nothing to save";
        return; // Nothing to save
    }
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename;
    if(!name.isEmpty()) {
        filename = m_points.begin().value().getTime().toUTC().toString(Qt::ISODate)
                + " - " + name + ".gpx";
    } else {
        filename = m_points.begin().value().getTime().toUTC().toString(Qt::ISODate)
                + ".gpx";
    }
    qDebug()<<"File:"<<homeDir<<"/"<<subDir<<"/"<<filename;

    QDir home = QDir(homeDir);
    if(!home.exists(subDir)) {
        qDebug()<<"Directory does not exist, creating";
        if(home.mkdir(subDir)) {
            qDebug()<<"Directory created";
        } else {
            qDebug()<<"Directory creation failed, aborting";
            return;
        }
    }

    QSaveFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }

    QXmlStreamWriter xml;
    xml.setDevice(&file);
    xml.setAutoFormatting(true);    // Human readable output
    xml.writeStartDocument();
    xml.writeDefaultNamespace("http://www.topografix.com/GPX/1/1");
    xml.writeStartElement("gpx");
    xml.writeAttribute("version", "1.1");
    xml.writeAttribute("Creator", "Rena for Sailfish");

    if(!name.isEmpty() || !desc.isEmpty()) {
        xml.writeStartElement("metadata");
        if(!name.isEmpty()) {
            xml.writeTextElement("name", name);
        }
        if(!desc.isEmpty()) {
            xml.writeTextElement("desc", desc);
        }
        xml.writeEndElement(); // metadata
    }

    xml.writeStartElement("trk");
    xml.writeStartElement("trkseg");

    for(QMap<uint, TrackPoint>::iterator i = m_points.begin(); i != m_points.end(); i++) {
        xml.writeStartElement("trkpt");
        xml.writeAttribute("lat", QString::number(i.value().hasCoordinate() ? i.value().getLatitude() : 0, 'g', 15));
        xml.writeAttribute("lon", QString::number(i.value().hasCoordinate() ? i.value().getLongitude() : 0, 'g', 15));

        xml.writeTextElement("time", i.value().getTime().toUTC().toString(Qt::ISODate));
        if(i.value().hasElevation()) {
            xml.writeTextElement("ele", QString::number(i.value().getElevation(), 'g', 15));
        }

        xml.writeStartElement("extensions");
        if(i.value().hasDirection()) {
            xml.writeTextElement("dir", QString::number(i.value().getDirection(), 'g', 15));
        }
        if(i.value().hasGroundSpeed()) {
            xml.writeTextElement("g_spd", QString::number(i.value().getGroundSpeed(), 'g', 15));
        }
        if(i.value().hasVerticalSpeed()) {
            xml.writeTextElement("v_spd", QString::number(i.value().getVerticalSpeed(), 'g', 15));
        }
        if(i.value().hasMagneticVariation()) {
            xml.writeTextElement("m_var", QString::number(i.value().getMagneticVariation(), 'g', 15));
        }
        if(i.value().hasHorizontalAccuracy()) {
            xml.writeTextElement("h_acc", QString::number(i.value().getHorizontalAccuracy(), 'g', 15));
        }
        if(i.value().hasVerticalAccuracy()) {
            xml.writeTextElement("v_acc", QString::number(i.value().getVerticalAccuracy(), 'g', 15));
        }
        if(i.value().hasDistance()) {
            xml.writeTextElement("distance", QString::number(i.value().getDistance(), 'g', 15));
        }
        if(i.value().hasCadence()) {
            xml.writeTextElement("cadence", QString::number(i.value().getCadence(), 'g', 15));
        }
        xml.writeEndElement(); // extensions

        xml.writeEndElement(); // trkpt
    }

    xml.writeEndElement(); // trkseg
    xml.writeEndElement(); // trk

    xml.writeEndElement(); // gpx
    xml.writeEndDocument();

    file.commit();
    if(file.error()) {
        qDebug()<<"Error in writing to a file";
        qDebug()<<file.errorString();
    } else {
        QDir renaDir = QDir(homeDir + "/" + subDir);
        renaDir.remove("Autosave");
		if (plugins) {
			qDebug() << "got plugins for uploading ttrack";
			plugins->uploadTrack(filename);
		} else {
			qDebug() << "didn't get plugins for uploading track";
		}
    }
}

void TrackRecorder::clearTrack() {
    m_points.clear();
    m_distance = 0;
    last_position_time = 0;
    last_distance_time = 0;
    m_isEmpty = true;

    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QDir renaDir = QDir(homeDir + "/" + subDir);
    renaDir.remove("Autosave");

    emit distanceChanged();
    emit timeChanged();
    emit isEmptyChanged();
    emit pointsChanged();
}

qreal TrackRecorder::accuracy() const {
    return m_accuracy;
}

int TrackRecorder::points() const {
    return m_points.size();
}

qreal TrackRecorder::distance() const {
    return m_distance;
}

QString TrackRecorder::time() const {
    uint hours, minutes, seconds;

    if(m_points.size() < 2) {
        hours = 0;
        minutes = 0;
        seconds = 0;
    } else {
        QDateTime first = m_points.begin().value().getTime();
        QDateTime last = (m_points.end()-1).value().getTime();
        qint64 difference = first.secsTo(last);
        hours = difference / (60*60);
        minutes = (difference - hours*60*60) / 60;
        seconds = difference - hours*60*60 - minutes*60;
    }

    QString timeStr = QString("%1h %2m %3s")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));

    return timeStr;
}

bool TrackRecorder::isTracking() const {
    return m_tracking;
}

void TrackRecorder::setIsTracking(bool tracking) {
    if(m_tracking == tracking) {
        return; // No change
    }
    m_tracking = tracking;

    if(m_posSrc) {  // If we have positioning
        if(m_tracking && !m_applicationActive) {
            // Start tracking when application at background -> positioning has to be enabled
            m_posSrc->startUpdates();
        }
        if(!m_tracking && !m_applicationActive) {
            // Stop tracking when application at background -> disable positioning
            m_posSrc->stopUpdates();
        }
    }

    emit isTrackingChanged();
}

bool TrackRecorder::isEmpty() const {
    return m_isEmpty;
}

bool TrackRecorder::applicationActive() const {
    return m_applicationActive;
}

void TrackRecorder::setApplicationActive(bool active) {
    if(m_applicationActive == active) {
        return; // No change
    }
    m_applicationActive = active;

    if(m_posSrc) {  // If we have positioning
        if(m_applicationActive && !m_tracking) {
            // Application became active without tracking
            m_posSrc->startUpdates();
        }
        if(!m_applicationActive && !m_tracking) {
            // Application went to background without tracking
            m_posSrc->stopUpdates();
        }
    }

    emit applicationActiveChanged();
}

QGeoCoordinate TrackRecorder::currentPosition() const {
    return m_currentPosition;
}

int TrackRecorder::updateInterval() const {
    return m_posSrc->updateInterval();
}

void TrackRecorder::setUpdateInterval(int updateInterval) {
    if(!m_posSrc) {
        qDebug()<<"Can't set update interval, position source not initialized!";
        return;
    }
    m_posSrc->setUpdateInterval(updateInterval);
    qDebug()<<"Setting update interval to"<<updateInterval<<"msec";
    emit updateIntervalChanged();
}

QGeoCoordinate TrackRecorder::trackPointAt(int index) {
    if(index < m_points.size()) {
		TrackPoint p = (m_points.begin()+index).value();
		QGeoCoordinate coord;
		if (p.hasCoordinate()) {
			coord.setLatitude(p.getLatitude());
			coord.setLongitude(p.getLongitude());
		}
		if (p.hasElevation()) {
			coord.setAltitude(p.getElevation());
		}
        return coord;
    } else {
        return QGeoCoordinate();
    }
}

int TrackRecorder::fitZoomLevel(int width, int height) {
    if(m_points.size() < 2 || width < 1 || height < 1) {
        // One point track or zero size map
        return 20;
    }

    // Keep also current position in view
    qreal minLon = qMin(m_minLon, (qreal)m_currentPosition.longitude());
    qreal maxLon = qMax(m_maxLon, (qreal)m_currentPosition.longitude());
    qreal minLat = qMin(m_minLat, (qreal)m_currentPosition.latitude());
    qreal maxLat = qMax(m_maxLat, (qreal)m_currentPosition.latitude());

    qreal trackMinX = (minLon + 180) / 360;
    qreal trackMaxX = (maxLon + 180) / 360;
    qreal trackMinY = sqrt(1-qLn(minLat*M_PI/180 + 1/qCos(minLat*M_PI/180))/M_PI);
    qreal trackMaxY = sqrt(1-qLn(maxLat*M_PI/180 + 1/qCos(maxLat*M_PI/180))/M_PI);

    qreal coord, pixel;
    qreal trackAR = qAbs((trackMaxX - trackMinX) / (trackMaxY - trackMinY));
    qreal windowAR = (qreal)width/(qreal)height;
    if(trackAR > windowAR ) {
        // Width limits
        coord = qAbs(trackMaxX - trackMinX);
        pixel = width;
    } else {
        // height limits
        coord = qAbs(trackMaxY - trackMinY);
        pixel = height;
    }

    // log2(x) = ln(x)/ln(2)
    int z = qFloor(qLn(pixel/256.0 * 1.0/coord * qCos((m_minLat+m_maxLat)/2*M_PI/180))
                   / qLn(2)) + 1;
    return z;
}

QGeoCoordinate TrackRecorder::trackCenter() {
    // Keep also current position in view
    qreal minLon = qMin(m_minLon, (qreal)m_currentPosition.longitude());
    qreal maxLon = qMax(m_maxLon, (qreal)m_currentPosition.longitude());
    qreal minLat = qMin(m_minLat, (qreal)m_currentPosition.latitude());
    qreal maxLat = qMax(m_maxLat, (qreal)m_currentPosition.latitude());

    return QGeoCoordinate((minLat+maxLat)/2, (minLon+maxLon)/2);
}

void TrackRecorder::autoSave() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename = "Autosave";
    QDir home = QDir(homeDir);

    if(m_points.size() < 1) {
        // Nothing to save
        return;
    }

    qDebug()<<"Autosaving";

    if(!home.exists(subDir)) {
        qDebug()<<"Directory does not exist, creating";
        if(home.mkdir(subDir)) {
            qDebug()<<"Directory created";
        } else {
            qDebug()<<"Directory creation failed, aborting";
            return;
        }
    }
    QFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }
    QTextStream stream(&file);
    stream.setRealNumberPrecision(15);

	QMap<uint, TrackPoint>::iterator i = m_points.find(m_autoSavePosition);
	if (i == m_points.end() && m_points.size()) {
		m_autoSavePosition = m_points.begin().key();
		i = m_points.find(m_autoSavePosition);
	} else {
		i++;
	}
	while (i != m_points.end()) {
		if (i.value().hasCoordinate()) {
			stream<<i.value().getLatitude();
			stream<<" ";
			stream<<i.value().getLongitude();
			stream<<" ";
		} else {
			stream<<"nan nan ";
		}
		if (i.value().hasTime()) {
			stream<<i.value().getTime().toUTC().toString(Qt::ISODate);
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if(i.value().hasElevation()) {
			stream<<i.value().getElevation();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasDirection()) {
			stream<<i.value().getDirection();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasGroundSpeed()) {
			stream<<i.value().getGroundSpeed();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasVerticalSpeed()) {
			stream<<i.value().getVerticalSpeed();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasMagneticVariation()) {
			stream<<i.value().getMagneticVariation();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasHorizontalAccuracy()) {
			stream<<i.value().getHorizontalAccuracy();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasVerticalAccuracy()) {
			stream<<i.value().getVerticalAccuracy();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasDistance()) {
			stream<<i.value().getDistance();
			stream<<" ";
		} else {
			stream<<"nan ";
		}
		if (i.value().hasCadence()) {
			stream<<i.value().getCadence();
		} else {
			stream<<"nan";
		}
		stream<<'\n';
		m_autoSavePosition = i.key();
		i++;
	}
    stream.flush();
    file.close();
}

void TrackRecorder::loadAutoSave() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename = "Autosave";
    QFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.exists()) {
        qDebug()<<"No autosave found";
        return;
    }

    qDebug()<<"Loading autosave";

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }
    QTextStream stream(&file);

    while(!stream.atEnd()) {
        TrackPoint point;
        qreal lat, lon, temp;
        QString timeStr;
        stream>>lat>>lon;
        if (lat == lat && lon == lon) {
			point.setLatitude(lat);
			point.setLongitude(lon);
		}
		stream>>timeStr;
		if (timeStr != "nan") {
			point.setTime(QDateTime::fromString(timeStr,Qt::ISODate));
		}
		stream>>temp;
		if(temp == temp) {
			point.setElevation(temp);
		}
        stream>>temp;
        if(temp == temp) {  // If value is not nan
            point.setDirection(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setGroundSpeed(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setVerticalSpeed(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setMagneticVariation(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setHorizontalAccuracy(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setVerticalAccuracy(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setDistance(temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setCadence(temp);
        }
        stream.readLine(); // Read rest of the line, if any
        m_points[point.getTime().toTime_t()] = point;
        if (point.hasCoordinate()) {
			if(m_points.size() > 1) {
				if(point.getLatitude() < m_minLat) {
					m_minLat = point.getLatitude();
				} else if(point.getLatitude() > m_maxLat) {
					m_maxLat = point.getLatitude();
				}
				if(point.getLongitude() < m_minLon) {
					m_minLon = point.getLongitude();
				} else if(point.getLongitude() > m_maxLon) {
					m_maxLon = point.getLongitude();
				}
			} else {
				m_minLat = m_maxLat = point.getLatitude();
				m_minLon = m_maxLon = point.getLongitude();
			}
			QGeoCoordinate coord(point.getLatitude(), point.getLongitude());
			emit newTrackPoint(coord);
		}
    }
    if (m_points.size()) {
		m_autoSavePosition = (--m_points.end()).key();
	} else {
		m_autoSavePosition = 0;
	}
    file.close();

    qDebug()<<m_points.size()<<"track points loaded";

    emit pointsChanged();
    emit timeChanged();

    if(m_points.size() > 1) {
        for(QMap<uint, TrackPoint>::iterator i = ++m_points.begin(); i != m_points.end(); i++) {
			TrackPoint *p1 = &(i-1).value();
			TrackPoint *p2 = &i.value();
			if (p1->hasCoordinate() && p2->hasCoordinate()) {
				QGeoCoordinate coord1(p1->getLatitude(), p1->getLongitude());
				QGeoCoordinate coord2(p2->getLatitude(), p2->getLongitude());
				m_distance += coord1.distanceTo(coord2);
			} else if (p1->hasDistance() && p2->hasDistance()) {
				m_distance += p2->getDistance() - p1->getDistance();
			}
        }
        emit distanceChanged();
    }

    if(!m_points.isEmpty()) {
        m_isEmpty = false;
        emit isEmptyChanged();
    }
}
