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
  explicit TaskWidget(Task&& task, QWidget* parent = nullptr);

  void setTask(Task&& new_task);
  Task& task() { return task_; };

 private:
  Task task_;
};

#endif  // TASKWIDGET_H
