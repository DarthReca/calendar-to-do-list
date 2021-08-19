#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include "calendar_classes/calendarevent.h"
#include "CalendarClient/calendarclient.h"
#include "calendar_classes/calendar.h"
#include <QOAuth2AuthorizationCodeFlow>

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventForm(CalendarEvent* event, CalendarClient& client, Calendar& calendar, bool existing, QWidget *parent = nullptr);
    ~CreateEventForm();

private slots:
    void on_allDayBox_stateChanged(int arg1);
signals:
    void requestView();

private:
    Ui::CreateEventForm *ui;
    QPointer<CalendarEvent> event_;
    CalendarClient* client_;
    Calendar* calendar_;
    bool existing_;
};

#endif // CREATEEVENTFORM_H
