#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/calendarevent.h"
#include "createeventform.h"

class EventWidget : public QPushButton {
  Q_OBJECT

 public:
  explicit EventWidget(CalendarEvent& event, QWidget* parent = nullptr);

  void updateDisplayInfo();

  CalendarEvent& event() const { return event_; };

 private:
  CalendarEvent& event_;
};

#endif  // EVENTWIDGET_H
