#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include "calendar_classes/calendarevent.h"
#include "CalendarClient/calendarclient.h"
#include <QOAuth2AuthorizationCodeFlow>

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventForm(CalendarEvent* event, CalendarClient& client, QWidget *parent = nullptr);
    ~CreateEventForm();

private slots:
    void on_allDayBox_stateChanged(int arg1);

private:
    Ui::CreateEventForm *ui;
    CalendarEvent* event_;
    CalendarClient* client_;
};

#endif // CREATEEVENTFORM_H
