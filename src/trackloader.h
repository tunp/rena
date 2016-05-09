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

#ifndef TRACKLOADER_H
#define TRACKLOADER_H

#include <QObject>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#include "TrackPoint.h"

class TrackLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QDateTime time READ time NOTIFY timeChanged)
    Q_PROPERTY(QString timeStr READ timeStr NOTIFY timeChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString durationStr READ durationStr NOTIFY durationChanged)
    Q_PROPERTY(qreal distance READ distance NOTIFY distanceChanged)
    Q_PROPERTY(qreal speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(qreal maxSpeed READ maxSpeed NOTIFY maxSpeedChanged)
    Q_PROPERTY(qreal pace READ pace NOTIFY paceChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)

public:
    explicit TrackLoader(QObject *parent = 0);
    QString filename() const;
    void setFilename(QString filename);
    QString name();
    QString description();
    QDateTime time();
    QString timeStr();
    uint duration();
    QString durationStr();
    qreal distance();
    qreal speed();
    qreal maxSpeed();
    qreal pace();
    bool loaded();
    Q_INVOKABLE int trackPointCount();
    Q_INVOKABLE QGeoCoordinate trackPointAt(int index);
    Q_INVOKABLE TrackPoint trackPointAt2(int index);
    QDateTime trackPointTimeAt(int index);

    // Temporary "hacks" to get around misbehaving Map.fitViewportToMapItems()
    Q_INVOKABLE int fitZoomLevel(int width, int height);
    Q_INVOKABLE QGeoCoordinate center();

signals:
    void filenameChanged();
    void nameChanged();
    void descriptionChanged();
    void timeChanged();
    void durationChanged();
    void distanceChanged();
    void speedChanged();
    void maxSpeedChanged();
    void paceChanged();
    void loadedChanged();
    void trackChanged();

public slots:

private:

    void load();

    QList<TrackPoint> m_points;
    bool m_loaded;
    bool m_error;
    QString m_filename;
    QString m_name;
    QString m_description;
    QDateTime m_time;
    uint m_duration;
    qreal m_distance;
    qreal m_speed;
    qreal m_maxSpeed;
    qreal m_pace;
    QGeoCoordinate m_center;
};

#endif // TRACKLOADER_H
