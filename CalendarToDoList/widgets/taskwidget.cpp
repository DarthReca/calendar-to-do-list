#include "taskwidget.h"

TaskWidget::TaskWidget(Task &task, CalendarClient &client, Calendar &calendar,
                       QWidget *parent)
    : QPushButton(parent) {
  task_ = &task;
  client_ = &client;
  calendar_ = &calendar;

  QString start_time = task.getStartDateTime().toString("hh:mm");
  QString end_time = task.getEndDateTime().toString("hh:mm");
  QString text =
      task.all_day()
          ? task.summary()
          : QString("%1\n%2 - %3").arg(task.summary(), start_time, end_time);
  setText(text);
  setStyleSheet(
      QString("background-color: %2; color: white").arg(task.getColor()));
}
