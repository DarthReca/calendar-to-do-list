#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDomElement>
#include <QObject>

#include "icalendarcomponent.h"

class ICalendar {
 public:
  ICalendar();
  static ICalendar fromXmlResponse(QDomElement &xml);

  ~ICalendar();

  QString toICalendarObject();

  QList<ICalendarComponent> &components() { return components_; };

  QString getDisplayName() const { return display_name_; };
  void setDisplayName(const QString &name) { display_name_ = name; };

 private:
  QString display_name_;
  QList<ICalendarComponent> components_;
};

#endif  // CALENDAR_H
