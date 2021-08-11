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
    ui->calendarTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
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

void MainWindow::on_createEvent_clicked()
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


void MainWindow::on_receiveChanges_clicked()
{
    CalendarClient_CalDAV::receiveChanges(*auth->google);
}


void MainWindow::on_actionGiorno_triggered()
{
   ui->calendarTable->setColumnCount(1);
}


void MainWindow::on_actionSettimanale_triggered()
{
  ui->calendarTable->setColumnCount(7);
}


void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
  QDate d(date);
  for(int i = 0; i < ui->calendarTable->columnCount(); i++)
  {
      QTableWidgetItem *item = new QTableWidgetItem(d.addDays(i).toString("dd"));
      ui->calendarTable->setHorizontalHeaderItem(i, item);
  }
}

const QList<CalendarEvent *> &MainWindow::showing_events() const
{
    return showing_events_;
}

void MainWindow::setShowing_events(const QList<CalendarEvent *> &newShowing_events)
{
    if (showing_events_ == newShowing_events)
        return;
    showing_events_ = newShowing_events;
    emit showing_eventsChanged();
}

