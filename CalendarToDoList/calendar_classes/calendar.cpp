#include "calendar.h"

Calendar::Calendar(QObject *parent) : QObject(parent) {
  events_ = QVector<CalendarEvent>();
  tasks_ = QVector<Task>();
}

Calendar::Calendar(const QString &href, const QString &eTag,
                   QTextStream &ical_object, QObject *parent) {
  for (QString line = ical_object.readLine(); !line.isNull();
       line = ical_object.readLine()) {
    if (line.contains("BEGIN:VEVENT")) {
      CalendarEvent event = CalendarEvent(ical_object, this);
      event.setHREF(href);
      event.setETag(eTag);
      if (event.summary() != "") {
        events_.append(event);
      }
    } else if (line.contains("BEGIN:VTODO")) {
      Task task = Task(ical_object, this);
      task.setHREF(href);
      task.setETag(eTag);
      if (task.summary() != "") {
        tasks_.append(task);
      }
    } else if (line.contains("CALNAME:")) {
      display_name_ = line.split(":")[1];
    }
  }
}

QString Calendar::ToICalendarObject() {
  QString ical_object = "BEGIN:VCALENDAR\r\n";
  for (CalendarEvent e : events_) ical_object.append(e.toiCalendar());
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

QList<Task> &Calendar::tasks() { return tasks_; }

void Calendar::setTasks(const QVector<Task> &newTasks) { tasks_ = newTasks; }
