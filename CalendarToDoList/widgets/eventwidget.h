#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "calendar_classes/calendarevent.h"
#include "createeventform.h"

class EventWidget : public QPushButton
{
    Q_OBJECT

public:
    explicit EventWidget(CalendarEvent& event, CalendarClient& client, QWidget *parent = nullptr);

private:
    QPointer<CalendarEvent> event_;
    QPointer<CalendarClient> client_;
};

#endif // EVENTWIDGET_H
