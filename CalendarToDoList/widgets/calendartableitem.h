#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/icalendarcomponent.h"
#include "createeventform.h"

class CalendarTableItem : public QPushButton {
 public:
  explicit CalendarTableItem(ICalendarComponent&& component,
                             QWidget* parent = nullptr);

  void setComponent(ICalendarComponent&& new_component);
  ICalendarComponent& component() { return component_; };

 private:
  ICalendarComponent component_;
};

#endif  // EVENTWIDGET_H
