#include "calendar.h"

Calendar::Calendar(QObject *parent) : QObject(parent) {
  events_ = QVector<CalendarEvent>();
  task_lists_ = QList<TaskList>();
}

Calendar::Calendar(const QString &href, const QString &eTag,
                   QTextStream &ical_object, QObject *parent) {
  QString line = ical_object.readLine();

  while (!line.isNull()) {
    if (line.contains("BEGIN:VEVENT")) {
      CalendarEvent event = CalendarEvent(ical_object, this);
      event.setHREF(href);
      event.setETag(eTag);
      if (event.summary() != "") {
        events_.append(event);
      }
    }
    if (line.contains("CALNAME:")) {
      display_name_ = line.split(":")[1];
    }
    line = ical_object.readLine();
  }
}

QString Calendar::ToICalendarObject() {
  QString ical_object = "BEGIN:VCALENDAR\r\n";
  for (CalendarEvent e : events_) ical_object.append(e.ToICalendarObject());
  ical_object.append("END:VCALENDAR");
  return ical_object;
}

Calendar::~Calendar() { events_.clear(); }

QString Calendar::getDisplayName(void) const { return display_name_; }

void Calendar::setDisplayName(QString name) {
  display_name_ = name;
  emit displayNameChanged(display_name_);
}

QVector<CalendarEvent> &Calendar::events() { return events_; }

void Calendar::setEvents(const QVector<CalendarEvent> &newEvents) {
  events_ = newEvents;
}
