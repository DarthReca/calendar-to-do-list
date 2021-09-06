#ifndef TASK_H
#define TASK_H

#include <QColor>

#include "calendarevent.h"

class Task : public CalendarEvent {
 public:
  Task()
      : CalendarEvent(),
        completed_(QPair<bool, QDateTime>(false, QDateTime())) {
    setColor(QColor(Qt::blue).name());
  };

  Task &fromICalendar(QTextStream &icalendar) override { return *this; };
  QString toICalendar() override;

  void flipCompleted();

  const QPair<bool, QDateTime> &completed() const;

 private:
  QPair<bool, QDateTime> completed_;
};

#endif  // TASK_H
