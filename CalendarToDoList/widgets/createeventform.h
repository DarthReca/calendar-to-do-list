#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include <QJsonArray>
#include <memory>

#include "CalendarClient/calendarclient.h"
#include "calendar_classes/calendar.h"
#include "calendar_classes/calendarevent.h"
#include "calendar_classes/task.h"

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog {
  Q_OBJECT

 public:

  template <typename T>
  explicit CreateEventForm(T* event, CalendarClient& client,
                           ICalendar& calendar, bool existing,
                           QWidget* parent = nullptr);

  ~CreateEventForm();

  void resetFormFields();
  CalendarEvent getEvent() { return *event_; }
 signals:
  void requestView();

 private:
  Ui::CreateEventForm* ui;
  CalendarEvent* event_;
  QPointer<CalendarClient> client_;
  ICalendar* calendar_;
  bool existing_;
};

#endif  // CREATEEVENTFORM_H
