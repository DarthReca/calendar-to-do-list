#ifndef USERFORM_H
#define USERFORM_H

#include <QDialog>
#include <QAbstractButton>
#include <QMessageBox>

namespace Ui {
class Userform;
}

class Userform : public QDialog
{
    Q_OBJECT

public:
    explicit Userform(QWidget *parent = nullptr);
    ~Userform();

private slots:

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::Userform *ui;
};

#endif // USERFORM_H
