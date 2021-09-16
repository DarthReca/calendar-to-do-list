#include "usercalendarschoice.h"
#include "ui_usercalendarschoice.h"

UserCalendarsChoice::UserCalendarsChoice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserCalendarsChoice)
{
    ui->setupUi(this);
}

UserCalendarsChoice::~UserCalendarsChoice()
{
    delete ui;
}
