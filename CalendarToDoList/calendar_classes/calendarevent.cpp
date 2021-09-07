#include "calendarevent.h"

#include <QColor>
#include <QDebug>
#include <QRandomGenerator>

CalendarEvent::CalendarEvent() {
  calendar_name_ = "unnamed";
  summary_ = "";
  location_ = "";
  description_ = "";
  start_date_time_ = QDateTime().currentDateTime();
  end_date_time_ = QDateTime().currentDateTime().addSecs(60 * 60);
  categories_ = "";
  exdates_ = "";
  RRULE_ = "";
  color_ = QColor(Qt::red).name();
  uid_ = "";
  href_ = "";
  eTag_ = "";
  all_day_ = false;
}

CalendarEvent& CalendarEvent::fromICalendar(QTextStream& icalendar) {
  for (QString line = icalendar.readLine(); !line.contains("END:VEVENT");
       line = icalendar.readLine()) {
    QStringList key_value = line.split(":");
    if (key_value.size() != 2) continue;
    QString key = key_value[0];
    QString value = key_value[1].replace("\\n", "\n");

    QString testEncodingString = value.toUtf8();
    if (!testEncodingString.contains("�")) value = testEncodingString;

    if (key.startsWith(QLatin1String("DTSTART"))) {
      setStartDateTime(dateTimeFromString(value));
    } else if (key.startsWith(QLatin1String("DTEND"))) {
      setEndDateTime(dateTimeFromString(value));
    } else if (key.startsWith(QLatin1String("DUE"))) {
      QDateTime date_time = dateTimeFromString(value);
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

QString CalendarEvent::toICalendar() {
  QString ical_object =
      "BEGIN:VEVENT\r\n"
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
      "DTSTART:" +
      (all_day_ ? start_date_time_.toUTC().toString("yyyyMMdd")
                : start_date_time_.toUTC().toString("yyyyMMdd'T'HHmmss'Z'")) +
      "\r\n"
      "DTEND:" +
      (all_day_ ? end_date_time_.toUTC().toString("yyyyMMdd")
                : end_date_time_.toUTC().toString("yyyyMMdd'T'HHmmss'Z'")) +
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

int CalendarEvent::weekDayFromString(const QString& weekday_string) {
  int iRet = 0;

  if (weekday_string.endsWith("MO", Qt::CaseInsensitive)) {
    iRet = 1;
  } else if (weekday_string.endsWith("TU", Qt::CaseInsensitive)) {
    iRet = 2;
  } else if (weekday_string.endsWith("WE", Qt::CaseInsensitive)) {
    iRet = 3;
  } else if (weekday_string.endsWith("TH", Qt::CaseInsensitive)) {
    iRet = 4;
  } else if (weekday_string.endsWith("FR", Qt::CaseInsensitive)) {
    iRet = 5;
  } else if (weekday_string.endsWith("SA", Qt::CaseInsensitive)) {
    iRet = 6;
  } else if (weekday_string.endsWith("SU", Qt::CaseInsensitive)) {
    iRet = 7;
  }

  return iRet;
}

QString CalendarEvent::stringFromWeekDay(int weekday) {
  switch (weekday) {
    case 1:
      return "MO";
    case 2:
      return "TU";
    case 3:
      return "WE";
    case 4:
      return "TH";
    case 5:
      return "FR";
    case 6:
      return "SA";
    case 7:
      return "SU";
  }
  return "";
}

const QString& CalendarEvent::eTag() const { return eTag_; }

QList<QDateTime> CalendarEvent::recurrencesInRange(QDateTime from,
                                                   QDateTime to) {
  // Look at QList<QObject*> CalendarClient::eventsForDate(const QDate& date)
  QList<QDateTime> list;
  if (RRULE_.isEmpty()) {
    if (start_date_time_ >= from)
      list += start_date_time_;
    else if (start_date_time_ < from && end_date_time_ <= to)
      list += QDateTime(from.date(), QTime(0, 0));
  } else {
    auto rules_map = parseRRule();
    QList<QDateTime> tmp;

    QString freq = rules_map["FREQ"];
    QSet<int> by_day;

    QDateTime start = from < start_date_time_ ? start_date_time_ : from;
    if (freq == "DAILY") {
      for (QDateTime i = start; i < to; i = i.addDays(1)) tmp += i;
    }
    if (freq == "WEEKLY") {
      // Parse BYDAY
      for (QString& day : rules_map["BYDAY"].split(",", Qt::SkipEmptyParts))
        by_day += weekDayFromString(day);
      for (QDateTime i = start; i < to; i = i.addDays(1))
        if (by_day.contains(i.date().dayOfWeek())) tmp += i;
    }
    if (freq == "MONTHLY") {
      // Parse BYMONTHDAY
      int day_of_month = rules_map["BYMONTHDAY"].toInt();
      for (QDateTime i = start; i < to; i = i.addDays(1))
        if (i.date().day() == day_of_month) tmp += i;
    }
    if (freq == "YEARLY") {
      // Parse BYMONTH, BYMONTHDAY
      int day_of_month = rules_map["BYMONTHDAY"].toInt();
      int month = rules_map["BYMONTH"].toInt();
      if (month == 0) month = start_date_time_.date().month();
      if (day_of_month == 0) day_of_month = start_date_time_.date().day();
      for (QDateTime i = start; i < to; i = i.addDays(1))
        if (i.date().month() == month && i.date().day() == day_of_month)
          tmp += i;
    }
    list += tmp;
  }
  for (QDateTime& dt : list) dt.setTime(start_date_time_.time());
  return list;
}

QDateTime CalendarEvent::dateTimeFromString(const QString& date_time_string) {
  // UTC
  QDateTime date_time =
      QDateTime::fromString(date_time_string, "yyyyMMdd'T'hhmmss'Z'");
  date_time.setTimeSpec(Qt::UTC);
  qDebug() << date_time_string << " -> " << date_time;
  if (date_time.isValid()) return date_time.toLocalTime();

  // LocalTime
  date_time = QDateTime::fromString(date_time_string, "yyyyMMdd'T'hhmmss");
  date_time.setTimeSpec(Qt::LocalTime);
  if (date_time.isValid()) return date_time;

  date_time = QDateTime::fromString(date_time_string, "yyyyMMddhhmmss");
  if (date_time.isValid()) return date_time;

  // No Hour
  date_time = QDateTime::fromString(date_time_string, "yyyyMMdd");
  if (date_time.isValid()) return date_time;

  qWarning() << date_time_string << " is not parsable";
  return date_time;
}

/* Public slots */

QString CalendarEvent::getColor(void) const { return color_; }

void CalendarEvent::setColor(const QString& color) {
  if (color != color_) {
    color_ = color;
  }
}

QString CalendarEvent::calendarName() const { return calendar_name_; }

void CalendarEvent::setCalendarName(const QString& calendarName) {
  if (calendarName != calendar_name_) {
    calendar_name_ = calendarName;
  }
}

QString CalendarEvent::summary() const { return summary_; }

void CalendarEvent::setSummary(const QString& name) {
  if (name != summary_) {
    summary_ = name;
  }
}

QHash<QString, QString> CalendarEvent::parseRRule() {
  QStringList rruleList = RRULE_.trimmed().split(";", Qt::SkipEmptyParts);
  QHash<QString, QString> map;

  // divide into rule name and rule parameter(s)
  for (QString& ruleString : rruleList) {
    QStringList rruleElements =
        ruleString.trimmed().split("=", Qt::SkipEmptyParts);
    // result must have 2 elements
    if (rruleElements.length() != 2) {
      qDebug() << "ERROR: invalid rule element count" << rruleElements.length()
               << "in rule" << ruleString;
    }
    if (rruleElements.at(0).toUpper() == "FREQ") {
      map["FREQ"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "INTERVAL") {
      map["INTERVAL"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "COUNT") {
      map["COUNT"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "UNTIL") {
      map["UNTIL"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "WKST") {
      map["WKST"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYDAY") {
      map["BYDAY"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYMONTHDAY") {
      map["BYMONTHDAY"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYMONTH") {
      map["BYMONTH"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYYEAR") {
      map["BYYEAR"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYWEEKNO") {
      map["BYWEEKNO"] = rruleElements.at(1);
    } else if (rruleElements.at(0).toUpper() == "BYSETPOS") {
      map["BYSETPOS"] = rruleElements.at(1);
    } else {
      qDebug() << "WARNING: unsupported rrule element" << rruleElements.at(0);
    }
  }
  return map;
}

bool CalendarEvent::all_day() const { return all_day_; }

void CalendarEvent::setAllDay(bool newAll_day) { all_day_ = newAll_day; }

void CalendarEvent::setETag(const QString& newETag) { eTag_ = newETag; }

QString CalendarEvent::location() const { return location_; }

void CalendarEvent::setLocation(const QString& location) {
  if (location != location_) {
    location_ = location;
  }
}

QString CalendarEvent::description() const { return description_; }

void CalendarEvent::setDescription(const QString& description) {
  if (description != description_) {
    description_ = description;
  }
}

QDateTime CalendarEvent::startDateTime(void) const { return start_date_time_; }

void CalendarEvent::setStartDateTime(const QDateTime& startDateTime) {
  if (startDateTime != start_date_time_) {
    start_date_time_ = startDateTime;
  }
}

QDateTime CalendarEvent::endDateTime(void) const { return end_date_time_; }

void CalendarEvent::setEndDateTime(const QDateTime& endDateTime) {
  if (endDateTime != end_date_time_) {
    end_date_time_ = endDateTime;
  }
}

QString CalendarEvent::RRULE() const { return RRULE_; }

void CalendarEvent::setRRULE(const QString& rrule) {
  if (RRULE_ != rrule) {
    RRULE_ = rrule;
  }
}

QString CalendarEvent::exdates() const { return exdates_; }

void CalendarEvent::setExdates(const QString& exdates) {
  if (exdates_ != exdates) {
    exdates_ = exdates;
  }
}

QString CalendarEvent::categories() const { return categories_; }

void CalendarEvent::setCategories(const QString& categories) {
  if (categories_ != categories) {
    categories_ = categories;
  }
}

QString CalendarEvent::uid(void) const { return uid_; }

QString CalendarEvent::href(void) const { return href_; }

void CalendarEvent::setUid(const QString& uid) {
  if (uid != uid_) {
    uid_ = uid;
  }
}

void CalendarEvent::setHref(const QString& href) {
  if (href != href_) {
    href_ = href;
  }
}
