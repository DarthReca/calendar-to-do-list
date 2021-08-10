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
    auto layout = ui->gridLayout;
    EventWidget *event_button = new EventWidget(event, this);
    layout->addWidget(event_button);
}
void MainWindow::on_addButton_clicked()
{
   CalendarEvent event(nullptr);
   event.setDescription("Random Desc");
   event.setStartDateTime(QDateTime::currentDateTime());
   event.setEndDateTime(QDateTime::currentDateTime().addSecs(60*60));
   on_request_event_ui(event);
}

