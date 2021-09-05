#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent& event, QWidget* parent)
    : QPushButton(parent) {
  event_ = &event;

  QString start_time = event.getStartDateTime().toString("hh:mm");
  QString end_time = event.getEndDateTime().toString("hh:mm");
  QString text =
      event.all_day()
          ? event.summary()
          : QString("%1\n%2 - %3").arg(event.summary(), start_time, end_time);
  setText(text);
  setStyleSheet(
      QString("background-color: %1; color: white").arg(event.getColor()));
}
