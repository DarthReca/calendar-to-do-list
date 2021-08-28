#include "task.h"

QString Task::toVTodo() {
  QString ical_object =
      "BEGIN:VTODO\r\n"
      "UID:" +
      UID_ +
      "\r\n"
      "VERSION:2.0\r\n"
      "DTSTAMP:" +
      QDateTime::currentDateTime().toString("yyyyMMddTHHmmssZ") +
      "\r\n"
      "SUMMARY:" +
      summary_ +
      "\r\n"
      "DUE:" +
      (all_day_ ? start_date_time_.toString("yyyyMMdd")
                : start_date_time_.toString("yyyyMMddTHHmmss")) +
      "\r\n"
      "LOCATION:" +
      location_ +
      "\r\n"
      "DESCRIPTION:" +
      description_ +
      "\r\n"
      "TRANSP:OPAQUE\r\n";
  if (!RRULE_.isEmpty()) {
    ical_object.append("RRULE:" + RRULE_ + "\r\n");
  }

  if (!exdates_.isEmpty()) {
    ical_object.append("EXDATE:" + exdates_ + "\r\n");
  }
  ical_object.append("END:VEVENT\r\n");

  return ical_object;
}

void Task::FlipCompleted() {
  completed_.first = !completed_.first;
  completed_.second = QDateTime::currentDateTime();
}

const QPair<bool, QDateTime>& Task::completed() const { return completed_; }
