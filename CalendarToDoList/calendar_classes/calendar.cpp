#include "calendar.h"

ICalendar::ICalendar() { components_ = QList<ICalendarComponent>(); }

ICalendar::ICalendar(const QString &href, const QString &eTag,
                     QTextStream &ical_object)
    : ICalendar() {
  for (QString line = ical_object.readLine(); !line.isNull();
       line = ical_object.readLine()) {
    if (line.contains("CALNAME:")) {
      display_name_ = line.split(":")[1];
    } else if (line.contains("VEVENT") || line.contains("VTODO")) {
      QString type = line.split(":")[1];
      ICalendarComponent component =
          ICalendarComponent::fromICalendar(ical_object, type);
      component.setHref(href);
      component.setEtag(eTag);
      components_ += component;
    }
  }
}

QString ICalendar::toICalendarObject() {
  QString ical_object = "BEGIN:VCALENDAR\r\n";
  for (ICalendarComponent &comp : components_)
    ical_object += comp.toICalendar();
  ical_object += "END:VCALENDAR";
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

ICalendar::~ICalendar() { components_.clear(); }

QString ICalendar::getDisplayName(void) const { return display_name_; }

void ICalendar::setDisplayName(QString name) { display_name_ = name; }
