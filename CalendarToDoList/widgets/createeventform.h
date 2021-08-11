#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>
#include "calendar_classes/calendarevent.h"

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventForm(CalendarEvent* event, QWidget *parent = nullptr);
    ~CreateEventForm();

private slots:
    void on_titleEdit_textChanged(const QString &arg1);

private:
    Ui::CreateEventForm *ui;
    CalendarEvent* event_;
};

#endif // CREATEEVENTFORM_H
