#ifndef USERCALENDARSCHOICE_H
#define USERCALENDARSCHOICE_H

#include <QAbstractButton>
#include <QDialog>

#include "calendarclient.h"

namespace Ui {
class UserCalendarsChoice;
}

class UserCalendarsChoice : public QDialog {
  Q_OBJECT

 public:
  explicit UserCalendarsChoice(CalendarClient &client,
                               QWidget *parent = nullptr);
  ~UserCalendarsChoice();

 private slots:
  void on_buttonBox_clicked(QAbstractButton *button);

 private:
  Ui::UserCalendarsChoice *ui;
  CalendarClient *formClient_;
};

#endif  // USERCALENDARSCHOICE_H
