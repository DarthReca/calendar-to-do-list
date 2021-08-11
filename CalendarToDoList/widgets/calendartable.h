#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QWidget>

namespace Ui {
class CalendarTable;
}

class CalendarTable : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarTable(QWidget *parent = nullptr);
    ~CalendarTable();

private:
    Ui::CalendarTable *ui;
};

#endif // CALENDARTABLE_H
