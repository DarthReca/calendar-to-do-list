#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/calendarevent.h"
#include "createeventform.h"

class EventWidget : public QPushButton {
  Q_OBJECT

 public:
  explicit EventWidget(CalendarEvent& event, CalendarClient& client,
                       Calendar& calendar, QWidget* parent = nullptr);

 private:
  QPointer<CalendarEvent> event_;
  QPointer<CalendarClient> client_;
  QPointer<Calendar> calendar_;
};

#endif  // EVENTWIDGET_H
