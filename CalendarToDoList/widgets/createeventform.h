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
  explicit CreateEventForm(CalendarEvent* event, CalendarClient& client,
                           Calendar& calendar, bool existing, bool isEvent,
                           QWidget* parent = nullptr);

  ~CreateEventForm();

  void resetFormFields();
 signals:
  void requestView();

 private:
  Ui::CreateEventForm* ui;
  CalendarEvent* event_;
  QPointer<CalendarClient> client_;
  QPointer<Calendar> calendar_;
  bool existing_;
  bool isEvent_;
};

#endif  // CREATEEVENTFORM_H
