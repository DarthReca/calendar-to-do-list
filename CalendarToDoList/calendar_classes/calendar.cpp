#include "calendar.h"

ICalendar::ICalendar() { events_ = QList<CalendarEvent>(); }

ICalendar::ICalendar(const QString &href, const QString &eTag,
                     QTextStream &ical_object)
    : ICalendar() {
  for (QString line = ical_object.readLine(); !line.isNull();
       line = ical_object.readLine()) {
    if (line.contains("BEGIN:VEVENT")) {
      CalendarEvent event = CalendarEvent().fromICalendar(ical_object);
      event.setHref(href);
      event.setETag(eTag);
      if (event.summary() != "") events_.append(event);
    } else if (line.contains("BEGIN:VTODO")) {
      Task task = Task().fromICalendar(ical_object);
      task.setHref(href);
      task.setETag(eTag);
      if (task.summary() != "") tasks_ += task;
    } else if (line.contains("CALNAME:")) {
      display_name_ = line.split(":")[1];
    }
  }
}

QString ICalendar::toICalendarObject() {
  QString ical_object = "BEGIN:VCALENDAR\r\n";
  for (CalendarEvent e : events_) ical_object.append(e.toICalendar());
  for (Task t : tasks_) ical_object += t.toICalendar();
  ical_object.append("END:VCALENDAR");
  return ical_object;
}

ICalendar &ICalendar::fromXmlResponse(QDomElement &xml) {
  QString icalendar =
      xml.elementsByTagName("cal:calendar-data").at(0).toElement().text();
  QString etag = xml.elementsByTagName("d:getetag").at(0).toElement().text();
  QString href = xml.elementsByTagName("d:href").at(0).toElement().text();
  QTextStream stream(&icalendar);
  *this = ICalendar(href, etag, stream);
  return *this;
}

ICalendar::~ICalendar() { events_.clear(); }

QString ICalendar::getDisplayName(void) const { return display_name_; }

void ICalendar::setDisplayName(QString name) { display_name_ = name; }

QVector<CalendarEvent> &ICalendar::events() { return events_; }

void ICalendar::setEvents(const QVector<CalendarEvent> &newEvents) {
  events_ = newEvents;
}
