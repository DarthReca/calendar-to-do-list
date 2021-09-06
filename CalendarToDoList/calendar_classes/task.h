#ifndef TASK_H
#define TASK_H

#include <QColor>
#include <QJsonObject>
#include <QObject>

#include "calendarevent.h"

class Task : public CalendarEvent {
  Q_OBJECT
 public:
  Task(QObject *parent = nullptr)
      : CalendarEvent(parent),
        completed_(QPair<bool, QDateTime>(false, QDateTime())) {
    setColor(QColor(Qt::blue).name());
  };
  Task(QTextStream& ical_object, QObject *parent = nullptr)
      : CalendarEvent(ical_object, parent),
        completed_(QPair<bool, QDateTime>(false, QDateTime())) {
    setColor(QColor(Qt::blue).name());
  };

  QString toiCalendar() override;
  void FlipCompleted();

  const QPair<bool, QDateTime> &completed() const;

 private:
  QPair<bool, QDateTime> completed_;
};

#endif  // TASK_H
