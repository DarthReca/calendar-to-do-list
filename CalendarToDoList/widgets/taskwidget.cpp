#include "taskwidget.h"

TaskWidget::TaskWidget(Task &&task, QWidget *parent) : QPushButton(parent) {
  setTask(std::move(task));
}

void TaskWidget::setTask(Task &&new_task) {
  task_ = new_task;
  QString start_time = task_.startDateTime().toString("hh:mm");
  QString end_time = task_.endDateTime().toString("hh:mm");
  QString text = QString("%1\t%2").arg(task_.summary(), start_time);
  if (task_.all_day()) text = task_.summary();

  setText(text);
  setStyleSheet(
      QString("background-color: %1; color: white").arg(task_.getColor()));
}
