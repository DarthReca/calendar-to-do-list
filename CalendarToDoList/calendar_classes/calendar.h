#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>

#include "calendarevent.h"
#include "task.h"

class ICalendar {
 public:
  ICalendar();
  ICalendar(const QString &href, const QString &eTag, QTextStream &ical_object);
  ~ICalendar();

  QString toICalendarObject();

  QList<CalendarEvent> &events();
  void setEvents(const QVector<CalendarEvent> &newEvents);

  QString getDisplayName() const;
  void setDisplayName(QString name);

 private:
  QString display_name_;
  QList<CalendarEvent> events_;
};

#endif  // CALENDAR_H
