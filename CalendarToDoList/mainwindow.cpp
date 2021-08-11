#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include "widgets/eventwidget.h"
#include <QLabel>
#include "CalendarClient/CalendarClient_CalDAV.h"

using namespace std;

GoogleAuth* auth;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Starting...\n";
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLogin_triggered()
{
    auth = new GoogleAuth(this);
}

void MainWindow::on_request_event_ui(CalendarEvent event)
{

}

void MainWindow::on_createEvent_clicked()
{
    editing_event_ = new CalendarEvent(nullptr);
    editing_event_->setUID("3nopsjhsq7dtugtjspkd1tlv91@google.com");
    CreateEventForm form(editing_event_, *auth->google,  this);
    form.exec();
    /*
    event.setDescription("Random Desc");
    event.setStartDateTime(QDateTime::currentDateTime());
    event.setEndDateTime(QDateTime::currentDateTime().addSecs(60*60));
    on_request_event_ui(event);
    */
}


void MainWindow::on_receiveChanges_clicked()
{
    CalendarClient_CalDAV::receiveChanges(*auth->google);
}

