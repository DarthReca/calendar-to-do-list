#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include "widgets/eventwidget.h"
#include <QLabel>
#include <QDomDocument>
#include "CalendarClient/CalendarClient_CalDAV.h"
#include <QApplication>

GoogleAuth* auth;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->calendarTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    qDebug() << "Starting...\n";

    // Force user to authenticate
    /*
    if(auth == nullptr)
        auth = new GoogleAuth(this);
    QEventLoop loop;
    connect(auth->google, &QOAuth2AuthorizationCodeFlow::granted, &loop, &QEventLoop::quit);
    loop.exec();
    */

    connect(this, &MainWindow::showing_eventsChanged, this, &MainWindow::on_showing_events_changed);
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
    CalendarEvent ev1(this);
    ev1.setName("SomeName");
    QList<CalendarEvent *> list;
    list.append(&ev1);

    setShowing_events(list);
    return;

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


/*void MainWindow::on_receiveChanges_clicked()
{
    auto reply = CalendarClient_CalDAV::requestSyncToken(*auth->google);
    connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
        QDomDocument q;
        q.setContent(reply->readAll());
        CalendarClient_CalDAV::receiveChanges(*auth->google, q.elementsByTagName("D:sync-token").at(0).toElement().text());
    });
}*/

void MainWindow::on_receiveChanges_clicked()
{

}


void MainWindow::on_seeIfChanged_clicked()
{
    auto reply = CalendarClient_CalDAV::obtainCTag(*auth->google);
    connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
        QDomDocument q;
        q.setContent(reply->readAll());
        QDomElement thisCTag = q.elementsByTagName("cs:getctag").at(0).toElement();;
        if(CalendarClient_CalDAV::getCTag().text().compare(thisCTag.text())==0){
            CalendarClient_CalDAV::lookForChanges(*auth->google);
        }
    });
}

void MainWindow::on_showing_events_changed()
{
   auto children = ui->calendarTable->children();
   int column_width = ui->calendarTable->columnWidth(0);
   int row_heigth = ui->calendarTable->rowHeight(0);

   int header_height = ui->calendarTable->horizontalHeader()->height();
   int header_width = ui->calendarTable->verticalHeader()->width();

   qDebug() << header_width;

   for(const auto& r : showing_events_)
   {
       EventWidget* widget = new EventWidget(r, ui->calendarTable);
       widget->resize(column_width, row_heigth);
       widget->move(0, header_height);
       ui->centralwidget->layout()->addWidget(widget);
   }
}


void MainWindow::on_actionGiorno_triggered()
{
   ui->calendarTable->setColumnCount(1);
   QDate d;
   d = ui->calendarWidget->selectedDate();
   QTableWidgetItem *item = new QTableWidgetItem(d.toString("ddd dd MMMM yyyy"));
   ui->calendarTable->setHorizontalHeaderItem(0, item);
}


void MainWindow::on_actionSettimanale_triggered()
{
  ui->calendarTable->setColumnCount(7);
  /*QDate d;
  d = ui->calendarWidget->firstDayOfWeek();
  for(int i = 0; i < ui->calendarTable->columnCount(); i++)
  {
      QTableWidgetItem *item = new QTableWidgetItem(d.addDays(i).toString("ddd dd MMMM yyyy"));
      ui->calendarTable->setHorizontalHeaderItem(i, item);
  }*/
}


void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
  QDate d(date);
  QTableWidgetItem *item;
  if(ui->calendarTable->columnCount()==1){
    item = new QTableWidgetItem(d.toString("ddd dd MMMM yyyy"));
    ui->calendarTable->setHorizontalHeaderItem(0, item);
  }
  else{
    for(int i = 0; i < ui->calendarTable->columnCount(); i++)
    {
        item = new QTableWidgetItem(d.addDays(i).toString("dd"));
        ui->calendarTable->setHorizontalHeaderItem(i, item);
    }
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

