#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent event, QWidget *parent) : QPushButton(parent)
{
    QString start_time = event.getStartDateTime().toString("hh:mm");
    QString end_time = event.getEndDateTime().toString("hh:mm");
    setText(event.description() + "\n" + start_time + " - " + end_time);
    QPalette palette = QPalette("red");
    setPalette(palette);
}
