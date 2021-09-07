#include "task.h"

Task &Task::fromICalendar(QTextStream &icalendar)
{
    for (QString line = icalendar.readLine(); !line.contains("END:VTODO");
         line = icalendar.readLine()) {
      QStringList key_value = line.split(":");
      if (key_value.size() != 2) continue;
      QString key = key_value[0];
      QString value = key_value[1].replace("\\n", "\n");

      QString testEncodingString = value.toUtf8();
      if (!testEncodingString.contains("�")) value = testEncodingString;

      if (key.startsWith(QLatin1String("DTSTART"))) {
        setStartDateTime(dateTimeFromString(value).toLocalTime());
      } else if (key.startsWith(QLatin1String("DTEND"))) {
        setEndDateTime(dateTimeFromString(value).toLocalTime());
      } else if (key.startsWith(QLatin1String("DUE"))) {
        QDateTime date_time = dateTimeFromString(value).toLocalTime();
        setStartDateTime(date_time);
        setEndDateTime(date_time);
      } else if (key == QLatin1String("RRULE")) {
        setRRULE(value);
      } else if (key == QLatin1String("EXDATE")) {
        setExdates(value);
      } else if (key == QLatin1String("SUMMARY")) {
        setSummary(value);
      } else if (key == QLatin1String("LOCATION")) {
        setLocation(value);
      } else if (key == QLatin1String("UID")) {
        setUid(value);
      } else if (key == QLatin1String("CATEGORIES")) {
        setCategories(value);
      } else if (key == QLatin1String("DESCRIPTION")) {
        setDescription(value);
      }
    }
    return *this;
}

QString Task::toICalendar() {
  QString ical_object =
      "BEGIN:VTODO\r\n"
      "UID:" +
      uid_ +
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
  ical_object.append("END:VTODO\r\n");

  return ical_object;
}

void Task::flipCompleted() {
  completed_.first = !completed_.first;
  completed_.second = QDateTime::currentDateTime();
}

const QPair<bool, QDateTime>& Task::completed() const { return completed_; }
