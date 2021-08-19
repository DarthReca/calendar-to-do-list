#include "eventwidget.h"

EventWidget::EventWidget(CalendarEvent& event, CalendarClient& client, Calendar& calendar, QWidget *parent) : QPushButton(parent)
{
    event_ = &event;
    client_ = &client;
    calendar_ = &calendar;

    QString start_time = event.getStartDateTime().toString("hh:mm");
    QString end_time = event.getEndDateTime().toString("hh:mm");
    setText(QString("%1\n%2 - %3").arg(event.name(), start_time, end_time));
    setStyleSheet(QString("background-color: %1; color: white").arg(event.getColor()));

    connect(this, &QPushButton::clicked, [this]() {
        CreateEventForm form(event_, *client_, *calendar_, true);
        form.exec();
    });
}
