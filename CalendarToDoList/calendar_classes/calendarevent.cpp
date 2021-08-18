#include <QDebug>
#include <QColor>
#include <QRandomGenerator>

#include "calendarevent.h"

CalendarEvent::CalendarEvent(QObject *parent) : QObject(parent)
{
  QRandomGenerator rng;
  calendar_name_    = "unnamed";
  name_            = "";
  location_        = "";
  description_     = "";
  start_date_time_   = QDateTime().currentDateTime();
  end_date_time_     = QDateTime().currentDateTime().addSecs(60*60);
  categories_      = "";
  exdates_         = "";
  RRULE_           = "";
  color_           = QColor(rng.generate() & 0xFF, rng.generate() & 0xFF, rng.generate() & 0xFF).name();
  UID_             = "";
  HREF_            = "";
  calendar_pointer_ = NULL;
}

CalendarEvent::CalendarEvent(const QString &href, QTextStream& ical_object, QObject *parent) : CalendarEvent(parent)
{
    //QString display_name = parent->property("displayName").toString();
    QString display_name = "Display";
    setColor(QColor(Qt::GlobalColor::blue).name());
    setCalendarName(display_name);
    setCalendarPointer(parent);
    setHREF(href);
    QString line;
    QDateTime utcTime;
    while (!(line = ical_object.readLine()).contains(QByteArray("END:VEVENT")))
    {
      //QDEBUG << m_DisplayName << ": " << line;

      const int deliminatorPosition = line.indexOf(QLatin1Char(':'));
      const QString key   = line.mid(0, deliminatorPosition);
      QString value = (line.mid(deliminatorPosition + 1, -1).replace("\\n", "\n")); //.toLatin1();
      QString testEncodingString = value.toUtf8();
      if (false == testEncodingString.contains("�"))
      {
        value = testEncodingString;
      }

      if (key.startsWith(QLatin1String("DTSTART")))
      {
        utcTime = QDateTime::fromString(value, "yyyyMMdd'T'hhmmss'Z'");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMdd'T'hhmmss");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMddhhmmss");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMdd");
        if (!utcTime.isValid())
          qDebug() << display_name << ": " << "could not parse" << line;

        setStartDateTime(utcTime.toLocalTime());
      }
      else if (key.startsWith(QLatin1String("DTEND")))
      {
        utcTime = QDateTime::fromString(value, "yyyyMMdd'T'hhmmss'Z'");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMdd'T'hhmmss");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMddhhmmss");
        if (!utcTime.isValid())
          utcTime = QDateTime::fromString(value, "yyyyMMdd");
        if (!utcTime.isValid())
          qDebug() << display_name << ": " << "could not parse" << line;

        setEndDateTime(utcTime.toLocalTime());
      }
      else if (key == QLatin1String("RRULE"))
      {
        setRRULE(value);
      }
      else if (key == QLatin1String("EXDATE"))
      {
        setExdates(value);
      }
      else if (key == QLatin1String("SUMMARY"))
      {
        setName(value);
      }
      else if (key == QLatin1String("LOCATION"))
      {
        setLocation(value);
      }
      else if (key == QLatin1String("UID"))
      {
        setUID(value);
      }
      else if (key == QLatin1String("CATEGORIES"))
      {
        setCategories(value);
      }
      else if (key == QLatin1String("DESCRIPTION"))
      {
        setDescription(value);
      }
    }
}

CalendarEvent::CalendarEvent(const CalendarEvent& other) : QObject()
{
    copyFrom(other);
}

QString CalendarEvent::ToICalendarObject()
{
    QString ical_object =   "BEGIN:VEVENT\r\n"
                            "UID:" + UID_ + "\r\n"
                            "VERSION:2.0\r\n"
                            "DTSTAMP:" + QDateTime::currentDateTime().toString("yyyyMMddTHHmmssZ") + "\r\n"
                            "SUMMARY:" + name_ + "\r\n"
                            "DTSTART:" + start_date_time_.toString("yyyyMMddTHHmmss") + "\r\n"
                            "DTEND:" + end_date_time_.toString("yyyyMMddTHHmmss") + "\r\n"
                            "LOCATION:" + location_ + "\r\n"
                            "DESCRIPTION:" + description_ + "\r\n"
                            "TRANSP:OPAQUE\r\n";
    if (!RRULE_.isEmpty())
    {
      ical_object.append("RRULE:" + RRULE_ + "\r\n");
    }

    if (!exdates_.isEmpty())
    {
      ical_object.append("EXDATE:" + exdates_ + "\r\n");
    }
    ical_object.append("END:VEVENT\r\n");

    return ical_object;
}

QList<QDateTime> CalendarEvent::RecurrencesInRange(QDateTime from, QDateTime to)
{
    //Look at QList<QObject*> CalendarClient::eventsForDate(const QDate& date)
    QList<QDateTime> list;
    if(RRULE_.isEmpty())
    {
      if(start_date_time_ >= from)
          list += start_date_time_;
    }
    else
    {

    }
    return list;
}

CalendarEvent& CalendarEvent::operator=(const CalendarEvent& other)
{
  copyFrom(other);
  return (*this);
}

bool CalendarEvent::operator<(const CalendarEvent& other) const
{
  return (this->start_date_time_ < other.start_date_time_);
}

void CalendarEvent::copyFrom(const CalendarEvent& other)
{
  setColor(other.color_);
  setCalendarName(other.calendar_name_);
  setName(other.name_);
  setLocation(other.location_);
  setDescription(other.description_);
  setStartDateTime(other.start_date_time_);
  setEndDateTime(other.end_date_time_);
  setCategories(other.categories_);
  setExdates(other.exdates_);
  setRRULE(other.RRULE_);
  setUID(other.UID_);
  setHREF(other.HREF_);
  setCalendarPointer(other.calendar_pointer_);
  setParent(other.parent());
}

/* Public slots */

QString CalendarEvent::getColor(void) const
{
  return color_;
}

void CalendarEvent::setColor(const QString& color)
{
  if (color != color_)
  {
    color_ = color;
    emit colorChanged(color_);
  }
}

QString CalendarEvent::calendarName() const
{
  return calendar_name_;
}

void CalendarEvent::setCalendarName(const QString &calendarName)
{
  if (calendarName != calendar_name_)
  {
    calendar_name_ = calendarName;
    emit calendarNameChanged(calendar_name_);
  }
}


QString CalendarEvent::name() const
{
  return name_;
}


void CalendarEvent::setName(const QString &name)
{
  if (name != name_)
  {
    name_ = name;
    emit nameChanged(name_);
  }
}

void CalendarEvent::setCalendarPointer(QObject* ptr)
{
  calendar_pointer_ = ptr;
}

QObject* CalendarEvent::getCalendarPointer(void) const
{
  return calendar_pointer_;
}


QString CalendarEvent::location() const
{
  return location_;
}

void CalendarEvent::setLocation(const QString& location)
{
  if (location != location_)
  {
    location_ = location;
    emit locationChanged(location_);
  }
}


QString CalendarEvent::description() const
{
  return description_;
}

void CalendarEvent::setDescription(const QString& description)
{
  if (description != description_)
  {
    description_ = description;
    emit descriptionChanged(description_);
  }
}


QDateTime CalendarEvent::getStartDateTime(void) const
{
    return start_date_time_;
}

void CalendarEvent::setStartDateTime(const QDateTime &startDateTime)
{
  if (startDateTime != start_date_time_)
  {
    start_date_time_ = startDateTime;
    emit startDateTimeChanged(start_date_time_);
  }
}


QDateTime CalendarEvent::getEndDateTime(void) const
{
  return end_date_time_;
}

void CalendarEvent::setEndDateTime(const QDateTime &endDateTime)
{
  if (endDateTime != end_date_time_)
  {
    end_date_time_ = endDateTime;
    emit endDateTimeChanged(end_date_time_);
  }
}


QString CalendarEvent::getRRULE() const
{
  return RRULE_;
}

void CalendarEvent::setRRULE(const QString& rrule)
{
  if (RRULE_ != rrule)
  {
    RRULE_ = rrule;
    emit rruleChanged(RRULE_);
  }
}


QString CalendarEvent::getExdates() const
{
  return exdates_;
}

void CalendarEvent::setExdates(const QString& exdates)
{
  if (exdates_ != exdates)
  {
    exdates_ = exdates;
    emit exdatesChanged(exdates_);
  }
}


QString CalendarEvent::getCategories() const
{
  return categories_;
}

void CalendarEvent::setCategories(const QString& categories)
{
  if (categories_ != categories)
  {
    categories_ = categories;
    emit categoriesChanged(categories_);
  }
}

QString CalendarEvent::getUID(void) const
{
  return UID_;
}


QString CalendarEvent::getHREF(void) const
{
  return HREF_;
}


void CalendarEvent::setUID(const QString& uid)
{
  if (uid != UID_)
  {
    UID_ = uid;
    emit uidChanged(UID_);
  }
}


void CalendarEvent::setHREF(const QString& href)
{
  if (href != HREF_)
  {
    HREF_ = href;
    emit hrefChanged(HREF_);
  }
}
