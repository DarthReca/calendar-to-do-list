#include "createeventform.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateEventForm)
{
    ui->setupUi(this);
}

CreateEventForm::~CreateEventForm()
{
    delete ui;
}
