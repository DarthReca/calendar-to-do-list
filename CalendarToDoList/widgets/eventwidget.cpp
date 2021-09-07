#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent& event, QWidget* parent)
    : QPushButton(parent), event_(event) {
  updateDisplayInfo();
}

void EventWidget::updateDisplayInfo() {
  QString start_time = event_.startDateTime().toString("hh:mm");
  QString end_time = event_.endDateTime().toString("hh:mm");
  QString text =
      event_.all_day()
          ? event_.summary()
          : QString("%1\n%2 - %3").arg(event_.summary(), start_time, end_time);
  setText(text);
  setStyleSheet(
      QString("background-color: %1; color: white").arg(event_.getColor()));
}
