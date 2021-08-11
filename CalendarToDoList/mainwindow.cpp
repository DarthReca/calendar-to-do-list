#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include "widgets/eventwidget.h"
#include <QLabel>

using namespace std;

GoogleAuth* auth;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Starting...\n";

    // Force user to authenticate
    if(auth == nullptr)
        auth = new GoogleAuth(this);
    QEventLoop loop;
    connect(auth->google, &QOAuth2AuthorizationCodeFlow::granted, &loop, &QEventLoop::quit);
    loop.exec();
}

MainWindow::~MainWindow()
{
    delete auth;
    delete ui;
}

void MainWindow::on_actionLogin_triggered()
{
    auth = new GoogleAuth(this);
}

void MainWindow::on_request_event_ui(CalendarEvent event)
{

}

void MainWindow::on_creaEvento_clicked()
{
    editing_event_ = new CalendarEvent(nullptr);
    CreateEventForm form(editing_event_, *auth->google,  this);
    form.exec();
    /*
    event.setDescription("Random Desc");
    event.setStartDateTime(QDateTime::currentDateTime());
    event.setEndDateTime(QDateTime::currentDateTime().addSecs(60*60));
    on_request_event_ui(event);
    */
}

