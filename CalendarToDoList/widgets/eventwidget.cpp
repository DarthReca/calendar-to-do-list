#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent& event, QWidget *parent) : QPushButton(parent)
{
    QString start_time = event.getStartDateTime().toString("hh:mm");
    QString end_time = event.getEndDateTime().toString("hh:mm");
    setText(event.name() + "\n" + start_time + " - " + end_time);
    setStyleSheet("color: red;");
}
