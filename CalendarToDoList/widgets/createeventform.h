#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include "calendar_classes/calendarevent.h"
#include <QOAuth2AuthorizationCodeFlow>

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventForm(CalendarEvent* event, QOAuth2AuthorizationCodeFlow& google, QWidget *parent = nullptr);
    ~CreateEventForm();

private slots:
    void on_allDayBox_stateChanged(int arg1);

private:
    Ui::CreateEventForm *ui;
    CalendarEvent* event_;
    QOAuth2AuthorizationCodeFlow *google_;
};

#endif // CREATEEVENTFORM_H
