#ifndef USERFORM_H
#define USERFORM_H

#include <QAbstractButton>
#include <QDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "calendarclient.h"

namespace Ui {
class Userform;
}

class Userform : public QDialog {
  Q_OBJECT

 public:
  explicit Userform(CalendarClient &client, QWidget *parent = nullptr);
  ~Userform();

 private slots:

  void on_buttonBox_clicked(QAbstractButton *button);

 private:
  Ui::Userform *ui;
  CalendarClient *formClient_;
};

#endif  // USERFORM_H
