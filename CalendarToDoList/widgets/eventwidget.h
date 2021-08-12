#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "calendar_classes/calendarevent.h"

class EventWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit EventWidget(CalendarEvent& event, QWidget *parent = nullptr);

signals:

};

#endif // EVENTWIDGET_H
