#ifndef USERCALENDARSCHOICE_H
#define USERCALENDARSCHOICE_H

#include <QDialog>

namespace Ui {
class UserCalendarsChoice;
}

class UserCalendarsChoice : public QDialog
{
    Q_OBJECT

public:
    explicit UserCalendarsChoice(QWidget *parent = nullptr);
    ~UserCalendarsChoice();

private:
    Ui::UserCalendarsChoice *ui;
};

#endif // USERCALENDARSCHOICE_H
