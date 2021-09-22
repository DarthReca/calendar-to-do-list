#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include <QJsonArray>
#include <memory>

#include "calendar_classes/calendar.h"
#include "calendar_classes/icalendarcomponent.h"
#include "calendarclient.h"

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog {
  Q_OBJECT

 public:
  explicit CreateEventForm(ICalendarComponent* event, CalendarClient& client,
                           bool existing, QWidget* parent = nullptr);

  ~CreateEventForm();

  void resetFormFields();
  ICalendarComponent* component() { return component_; }

 signals:
  void requestView();

 private:
  Ui::CreateEventForm* ui;
  ICalendarComponent* component_;
  QPointer<CalendarClient> client_;
  bool existing_;
};

#endif  // CREATEEVENTFORM_H
