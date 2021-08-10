#ifndef CREATEEVENTFORM_H
#define CREATEEVENTFORM_H

#include <QDialog>

namespace Ui {
class CreateEventForm;
}

class CreateEventForm : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEventForm(QWidget *parent = nullptr);
    ~CreateEventForm();

private:
    Ui::CreateEventForm *ui;
};

#endif // CREATEEVENTFORM_H
