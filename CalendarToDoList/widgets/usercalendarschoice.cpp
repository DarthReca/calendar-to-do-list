#include "usercalendarschoice.h"
#include "ui_usercalendarschoice.h"

UserCalendarsChoice::UserCalendarsChoice(CalendarClient& client, QWidget *parent) :
    QDialog(parent),
    formClient_(&client),
    ui(new Ui::UserCalendarsChoice)
{
    ui->setupUi(this);

    auto list = formClient_->getUserCalendarsList();
    for(auto it=list.begin(); it != list.end(); it++){
        ui->comboBox_Calendars->addItem(it.key());
    }
}

UserCalendarsChoice::~UserCalendarsChoice()
{
    delete ui;
}

void UserCalendarsChoice::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button->text()=="OK"){
        QString user = ui->comboBox_Calendars->currentText();
        QUrl endpoint = formClient_->getUserCalendarsList().value(user);
        formClient_->setEndpoint(endpoint);
    }
    else{
        exit(0);
    }
}

