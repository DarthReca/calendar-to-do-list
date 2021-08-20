#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>

#include "calendarevent.h"

class Calendar : public QObject {
  Q_OBJECT

 public:
  Calendar(QObject *parent = nullptr);
  Calendar(const QString &href, const QString &eTag, QTextStream &ical_object,
           QObject *parent = nullptr);
  ~Calendar();

  QString ToICalendarObject();

  QVector<CalendarEvent> &events();
  void setEvents(const QVector<CalendarEvent> &newEvents);
 signals:
  void displayNameChanged(QString display_name);
 public slots:
  QString getDisplayName(void) const;
  void setDisplayName(QString name);

 private:
  QString display_name_;
  QVector<CalendarEvent> events_;
};

#endif  // CALENDAR_H
