#include "calendar.h"

ICalendar::ICalendar() {}

QString ICalendar::toICalendarObject() {
  QString ical_object =
      "BEGIN:VCALENDAR\r\n"
      "VERSION:2.0\r\n"
      "PRODID:MRC_CALDAV_CLIENT\r\n";
  for (ICalendarComponent &comp : components_)
    ical_object += comp.toICalendar();
  ical_object += "END:VCALENDAR";
  return ical_object;
}

ICalendar ICalendar::fromXmlResponse(QDomElement &xml) {
  ICalendar result;
  QString icalendar =
      xml.elementsByTagName("cal:calendar-data").at(0).toElement().text();
  QString etag = xml.elementsByTagName("d:getetag").at(0).toElement().text();
  QString href = xml.elementsByTagName("d:href").at(0).toElement().text();
  QTextStream stream(&icalendar);
  for (QString line = stream.readLine(); !line.isNull();
       line = stream.readLine()) {
    if (line.contains("CALNAME:")) {
      result.display_name_ = line.split(":")[1];
    } else if (line.contains("VEVENT") || line.contains("VTODO")) {
      QString type = line.split(":")[1];
      ICalendarComponent component =
          ICalendarComponent::fromICalendar(stream, type);
      component.setHref(href);
      component.setEtag(etag);
      result.components_ += component;
    }
  }
  return result;
}

ICalendar::~ICalendar() { components_.clear(); }
