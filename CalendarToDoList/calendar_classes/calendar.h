#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>

#include "calendarevent.h"
#include "task.h"

class Calendar : public QObject {
  Q_OBJECT

 public:
  Calendar(QObject *parent = nullptr);
  Calendar(const QString &href, const QString &eTag, QTextStream &ical_object,
           QObject *parent = nullptr);
  ~Calendar();

  QString ToICalendarObject();

  QList<CalendarEvent> &events();
  void setEvents(const QVector<CalendarEvent> &newEvents);

  QList<TaskList> &taskLists() { return task_lists_; };
 signals:
  void displayNameChanged(QString display_name);
 public slots:
  QString getDisplayName(void) const;
  void setDisplayName(QString name);

 private:
  QString display_name_;
  QList<CalendarEvent> events_;
  QList<TaskList> task_lists_;
};

#endif  // CALENDAR_H
