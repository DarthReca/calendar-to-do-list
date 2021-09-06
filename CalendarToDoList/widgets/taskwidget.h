#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/calendarevent.h"
#include "calendar_classes/task.h"
#include "createeventform.h"

class TaskWidget : public QPushButton {
  Q_OBJECT

 public:
  explicit TaskWidget(Task& task, CalendarClient& client, Calendar& calendar,
                      QWidget* parent = nullptr);

  Task* task() const { return task_; };

 private:
  Task* task_;
  QPointer<CalendarClient> client_;
  QPointer<Calendar> calendar_;
};

#endif  // TASKWIDGET_H
