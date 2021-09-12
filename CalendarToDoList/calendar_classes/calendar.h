#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDomElement>
#include <QObject>

#include "calendarevent.h"
#include "task.h"

class ICalendar {
 public:
  ICalendar();
  ICalendar(const QString &href, const QString &eTag, QTextStream &ical_object);
  ~ICalendar();

  QString toICalendarObject();
  ICalendar &fromXmlResponse(QDomElement &xml);

  QList<CalendarEvent> &events();
  void setEvents(const QVector<CalendarEvent> &newEvents);

  QList<Task> &tasks() { return tasks_; };

  QString getDisplayName() const;
  void setDisplayName(QString name);

 private:
  QString display_name_;
  QList<CalendarEvent> events_;
  QList<Task> tasks_;
};

#endif  // CALENDAR_H
