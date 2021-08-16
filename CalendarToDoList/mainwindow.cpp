#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include "widgets/eventwidget.h"
#include <QLabel>
#include <QDomDocument>
#include "CalendarClient/calendarclient.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->calendarTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    qDebug() << "Starting...\n";

    // Force user to authenticate
    if(auth_.isNull())
        auth_ = new GoogleAuth(this);
    QEventLoop loop;
    connect(auth_->google, &QOAuth2AuthorizationCodeFlow::granted, &loop, &QEventLoop::quit);
    loop.exec();

    client_ = new CalendarClient(*auth_, this);

    //ottengo tutti gli eventi nel calendario
    auto reply = client_->obtainCTag();
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QDomDocument res;
        res.setContent(reply->readAll());
        auto lista = res.elementsByTagName("cs:getctag");
        client_->setCTag(lista.at(0).toElement());
    });
    reply = client_->getAllEvents();
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QDomDocument res;
        res.setContent(reply->readAll());
        auto lista = res.elementsByTagName("caldav:calendar-data");
        for(int i=0; i<lista.size(); i++){
            qDebug() << lista.at(i).toElement().text();
            qDebug() << "\n";
        }
        //salvo gli eTags per vedere i futuri cambiamenti
        auto eTags = res.elementsByTagName("D:getetag");
        for(int i=0; i<eTags.size(); i++){
             client_->addETag(eTags.at(i).toElement());
        }
    });
    //dovranno essere visualizzati nel calendario

    connect(this, &MainWindow::showing_eventsChanged, this, &MainWindow::on_showing_events_changed);
    connect(ui->testButton, &QPushButton::clicked, this, &MainWindow::refresh_calendar_events);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh_calendar_events()
{
   auto reply = client_->getAllEvents();
   connect(reply, &QNetworkReply::finished, [this, reply]() {
       QDomDocument res;
       res.setContent(reply->readAll());
       auto calendars = res.elementsByTagName("caldav:calendar-data");
       auto hrefs_list = res.elementsByTagName("D:href");
       QString href = hrefs_list.at(0).toElement().text();
       for(int i=0; i<calendars.size(); i++){
           QString el = calendars.at(i).toElement().text();
           QTextStream stream(&el);
           calendar_ = new Calendar(href, stream);
           setShowing_events(&calendar_->events());
       }
   });
}

void MainWindow::on_createEvent_clicked()
{
    editing_event_ = new CalendarEvent(nullptr);
    CreateEventForm form(editing_event_, *client_,  this);
    form.exec();
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

void MainWindow::on_seeIfChanged_clicked()
{
    auto reply = client_->obtainCTag();
    connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
        QDomDocument q;
        q.setContent(reply->readAll());
        QDomElement thisCTag = q.elementsByTagName("cs:getctag").at(0).toElement();;
        if(client_->getCTag().text().compare(thisCTag.text())==0){
            client_->lookForChanges();
        }
    });
}

void MainWindow::on_showing_events_changed()
{
   //Delete all previous widgets
   for(auto child : ui->calendarTable->children())
      if(qobject_cast<EventWidget *>(child) != nullptr)
        child->deleteLater();

   int column_width = ui->calendarTable->columnWidth(0);
   int row_heigth = ui->calendarTable->rowHeight(0);

   QDate selected_date = ui->calendarWidget->selectedDate();

   for(auto& event : *showing_events_)
   {
       EventWidget* widget = new EventWidget(event, ui->calendarTable->viewport());
       widget->resize(column_width, row_heigth);

       int x_pos = column_width*selected_date.daysTo(event.getStartDateTime().date());
       int y_pos = row_heigth + row_heigth*event.getStartDateTime().time().hour();
       widget->move(x_pos, y_pos);

       widget->show();
   }
}

QList<CalendarEvent> *MainWindow::showing_events() const
{
    return showing_events_;
}

void MainWindow::setShowing_events(QList<CalendarEvent> *newShowing_events)
{
    if (showing_events_ == newShowing_events)
        return;
    showing_events_ = newShowing_events;
    emit showing_eventsChanged();
}


void MainWindow::on_actionGiorno_triggered()
{
   ui->calendarTable->setColumnCount(1);
   QDate d = ui->calendarWidget->selectedDate();
   QTableWidgetItem *item = new QTableWidgetItem(d.toString("ddd\ndd"));
   ui->calendarTable->setHorizontalHeaderItem(0, item);
}


void MainWindow::on_actionSettimanale_triggered()
{
  ui->calendarTable->setColumnCount(7);
  QDate d = ui->calendarWidget->selectedDate();
  for(int i = 0; i < ui->calendarTable->columnCount(); i++)
  {
      QTableWidgetItem *item = new QTableWidgetItem(d.addDays(i).toString("ddd\ndd"));
      ui->calendarTable->setHorizontalHeaderItem(i, item);
  }
}


void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
  QDate d(date);
  QTableWidgetItem *item;
  if(ui->calendarTable->columnCount()==1){
    item = new QTableWidgetItem(d.toString("ddd\ndd"));
    ui->calendarTable->setHorizontalHeaderItem(0, item);
  }
  else{
    for(int i = 0; i < ui->calendarTable->columnCount(); i++)
    {
        item = new QTableWidgetItem(d.addDays(i).toString("ddd\ndd"));
        ui->calendarTable->setHorizontalHeaderItem(i, item);
    }
  }
}


void MainWindow::on_updateButton_clicked()
{
    auto reply = client_->obtainCTag();
        connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
          QDomDocument res;
          res.setContent(reply->readAll());
          auto newCTag = res.elementsByTagName("cs:getctag").at(0).toElement();
          if(newCTag == client_->getCTag()){
              qDebug() << "Calendar already up to date";
          }
          else{
              //per ora faccio la get di tutto, ma è meglio solo cambiare le cose nuove
              reply = client_->getAllEvents();
              connect(reply, &QNetworkReply::finished, [reply]() {
                  QDomDocument res;
                  res.setContent(reply->readAll());
                  auto lista = res.elementsByTagName("caldav:calendar-data");
                  for(int i=0; i<lista.size(); i++){
                      qDebug() << lista.at(i).toElement().text();
                      qDebug() << "\n";
                  }
              });
          }
    });
}

