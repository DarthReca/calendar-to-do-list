#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent&& event, QWidget* parent)
    : QPushButton(parent) {
  setEvent(std::move(event));
}

void EventWidget::setEvent(CalendarEvent&& event) {
  event_ = event;
  QString start_time = event_.startDateTime().toString("hh:mm");
  QString end_time = event_.endDateTime().toString("hh:mm");
  QString text =
      QString("%1\n%2 - %3").arg(event_.summary(), start_time, end_time);
  if (start_time == end_time)
    text = QString("%1\t%2").arg(event_.summary(), start_time);
  if (event_.all_day()) text = event_.summary();

  setText(text);
  setStyleSheet(
      QString("background-color: %1; color: white").arg(event_.getColor()));
}
