#include "calendartable.h"
#include "ui_calendartable.h"

CalendarTable::CalendarTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalendarTable)
{
    ui->setupUi(this);
}

CalendarTable::~CalendarTable()
{
    delete ui;
}
