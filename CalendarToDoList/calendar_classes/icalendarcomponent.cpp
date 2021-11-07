#include "icalendarcomponent.h"

ICalendarComponent::ICalendarComponent() : type_("VEVENT"), all_day_(false) {
  properties_["TRANSP"] = "OPAQUE";
  properties_["VERSION"] = "2.0";
  // TODO: add something random to UID
  properties_["UID"] = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

ICalendarComponent ICalendarComponent::fromICalendar(QTextStream& icalendar,
                                                     const QString& type) {
  ICalendarComponent component;
  component.setType(type);
  for (QString line = icalendar.readLine();
       !line.contains("END:" + component.type_); line = icalendar.readLine()) {
    QStringList key_value = line.split(":");
    if (key_value.size() != 2) continue;
    QString key = key_value[0].split(";")[0];
    QString value = key_value[1].replace("\\n", "\n");

    if (key == "BEGIN")
      component.type_ = value;
    else
      component.properties_[key] = value;
  }
  auto date_start = component.getProperty("DTSTART");
  if (date_start) component.all_day_ = date_start->length() == 8;

  return component;
}

QString ICalendarComponent::toICalendar() {
  QString ical_object =
      "BEGIN:" + type_ + "\r\n" +
      "DTSTAMP:" + QDateTime::currentDateTime().toString("yyyyMMddTHHmmssZ") +
      "\r\n";

  for (auto key_value = properties_.constKeyValueBegin();
       key_value != properties_.constKeyValueEnd(); key_value++){
      qDebug() << key_value->first + "   " + key_value->second;
    if (key_value->first != "RECURRENCE-ID")
      ical_object +=
          QString("%1:%2\r\n").arg(key_value->first, key_value->second);
  }

  ical_object += "END:" + type_ + "\r\n";

  qDebug() << ical_object;
  return ical_object;
}

QDateTime ICalendarComponent::dateTimeFromString(
    const QString& date_time_string) {
  // UTC
  QDateTime date_time =
      QDateTime::fromString(date_time_string, "yyyyMMdd'T'hhmmss'Z'");
  date_time.setTimeSpec(Qt::UTC);
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
  return date_time;
}

int ICalendarComponent::weekDayFromString(const QString& weekday_string) {
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

QString ICalendarComponent::stringFromWeekDay(int weekday) {
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

std::optional<QString> ICalendarComponent::getProperty(
    const QString& name) const {
  if (!properties_.contains(name)) return {};
  return properties_[name];
}

void ICalendarComponent::setProperty(const QString& key, const QString& value) {
  properties_[key] = value;
}

void ICalendarComponent::removeProperty(const QString& name) {
  properties_.remove(name);
}

std::optional<QDateTime> ICalendarComponent::getStartDateTime() {
  QString dtstart = getProperty("DTSTART").value_or("");
  if (dtstart.isEmpty()) return {};
  QDateTime date_time = dateTimeFromString(dtstart);
  return date_time;
}

void ICalendarComponent::setStartDateTime(const QDateTime& start) {
  QString date_string = start.date().toString("yyyyMMdd");
  if (!all_day_) date_string = start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'");
  setProperty("DTSTART", date_string);
}

std::optional<QDateTime> ICalendarComponent::getEndDateTime() {
  QString dtend = getProperty("DTEND").value_or("");
  if (dtend.isEmpty()) dtend = getProperty("DUE").value_or("");
  // TODO QString duration = getProperty("DURATION").value_or("");
  if (dtend.isEmpty()) return {};
  return dateTimeFromString(dtend);
}

void ICalendarComponent::setEndDateTime(const QDateTime& end) {
  QString date_string = end.date().toString("yyyyMMdd");
  if (!all_day_) date_string = end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'");
  if (type_ == "VEVENT")
  {
    setProperty("DTEND", date_string);
    removeProperty("DUE");
  }
  else
  {
    removeProperty("DTEND");
    setProperty("DUE", date_string);
  }
}

QString ICalendarComponent::getUID() {
  if (!properties_.contains("UID")) throw("UID is required");
  return properties_["UID"];
}

void ICalendarComponent::setAllDay(const bool& all_day) {
  all_day_ = all_day;
  auto dtstart = getStartDateTime();
  auto dtend = getEndDateTime();

  if (dtstart) setStartDateTime(dtstart.value());
  if (dtend) setEndDateTime(dtend.value());
}
