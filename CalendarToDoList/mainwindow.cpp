#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include "widgets/eventwidget.h"
#include <QLabel>
#include <QDomDocument>
#include "CalendarClient/CalendarClient.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->calendarTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->calendarTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    qDebug() << "Starting...\n";

    // Force user to authenticate
    if(auth_.isNull())
        auth_ = new GoogleAuth(this);
    QEventLoop loop;
    connect(auth_->google, &QOAuth2AuthorizationCodeFlow::granted, &loop, &QEventLoop::quit);
    loop.exec();

    connect(this, &MainWindow::showing_eventsChanged, this, &MainWindow::on_showing_events_changed);
    connect(ui->testButton, &QPushButton::clicked, this, &MainWindow::refresh_calendar_events);
    connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &MainWindow::refresh_calendar_events);

    client_ = new CalendarClient(*auth_, this);

    //ottengo il cTag e tutti gli eventi nel calendario
    auto reply = client_->obtainCTag();
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        QDomDocument res;
        res.setContent(reply->readAll());
        auto lista = res.elementsByTagName("cs:getctag");
        client_->setCTag(lista.at(0).toElement());        
    });

    refresh_calendar_events();

    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(on_actionSincronizza_triggered()));
    timer_->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh_calendar_events()
{
   auto reply = client_->getAllEvents();

   connect(reply, &QNetworkReply::finished, [this, reply]() {
       if(!calendar_.isNull())
            calendar_->events().clear();

       QDomDocument res;
       res.setContent(reply->readAll());

       auto calendars = res.elementsByTagName("caldav:calendar-data");
       auto hrefs_list = res.elementsByTagName("D:href");

       QString href = hrefs_list.at(0).toElement().text();
       for(int i=0; i<calendars.size(); i++){
           QString el = calendars.at(i).toElement().text();
           QTextStream stream(&el);
           QPointer<Calendar> tmp = new Calendar(href, stream);
           if(calendar_.isNull())
               calendar_ = tmp;
           else
               calendar_->events().append(tmp->events());
       }

       //salvo gli eTags per vedere i futuri cambiamenti
       //è una mappa di <href, eTag>
       auto eTags = res.elementsByTagName("D:getetag");
       for(int i=0; i<eTags.size(); i++){
           client_->addETag(hrefs_list.at(i).toElement().text(), eTags.at(i).toElement());
       }

       if(ui->calendarTable->columnCount() == 7)
         on_actionSettimanale_triggered();
       else
         on_actionGiorno_triggered();

       /*QTimer timer;
       connect(timer, &QTimer::timeout, MainWindow::on_actionSincronizza_triggered());
       timer.start(5000);*/
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
   for(auto child : ui->calendarTable->viewport()->children())
      if(qobject_cast<EventWidget *>(child) != nullptr)
        child->deleteLater();

   int column_width = ui->calendarTable->columnWidth(0);
   int row_heigth = ui->calendarTable->rowHeight(0);

   QDate selected_date = ui->calendarWidget->selectedDate();

   for(auto& event : *showing_events_)
   {
       EventWidget* widget = new EventWidget(event, *client_, ui->calendarTable->viewport());
       QTime start_time = event.getStartDateTime().time();
       int days_long = event.getStartDateTime().daysTo(event.getEndDateTime());

       int x_pos = column_width*selected_date.daysTo(event.getStartDateTime().date());
       int y_pos = row_heigth + row_heigth*( start_time.hour() + start_time.minute()/60.0);

       ui->calendarTable->scrollToTop();
       if(days_long == 0)
       {
           widget->resize(column_width, row_heigth);
           widget->move(x_pos, y_pos);
       }
       else
       {
           widget->move(x_pos, 0);
           widget->resize((days_long+1)*column_width, row_heigth);
       }

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
  updateTableToNDays(1);
}


void MainWindow::on_actionSettimanale_triggered()
{
  updateTableToNDays(7);
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

void MainWindow::on_actionSincronizza_triggered()
{
    auto reply = client_->obtainCTag();
        connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
          QDomDocument res;
          res.setContent(reply->readAll());
          auto newCTag = res.elementsByTagName("cs:getctag").at(0).toElement();

          if(newCTag.text() == client_->getCTag().text()){
              qDebug() << "Calendar already up to date";
          }
          else{
              client_->setCTag(newCTag);
              reply = client_->lookForChanges();
              connect(reply, &QNetworkReply::finished, [this, reply]() {

                  //creo una mappa con gli href e gli etag nuovi
                  QDomDocument res;
                  res.setContent(reply->readAll());
                  auto hrefs_list = res.elementsByTagName("D:href");
                  auto eTags = res.elementsByTagName("D:getetag");
                  QMap<QString, QDomElement> mapTmp;
                  for(int i=0; i<eTags.size(); i++){
                      mapTmp.insert(hrefs_list.at(i).toElement().text(), eTags.at(i).toElement());
                  }

                  //confronto la nuova mappa con quella esistente
                  //e aggiorno la lista di eTag nel client
                  QMap<QString, QDomElement> oldMap = client_->getETags();
                  QMap<QString, QDomElement>::iterator i;
                  for(i = oldMap.begin(); i != oldMap.end(); ++i){
                      if(mapTmp.contains(i.key())){
                          if(mapTmp[i.value().text()]!=oldMap[i.value().text()]){
                              qDebug() << "Item with href " + i.key() + "has a new etag: " + i.value().text() + "\n\n";
                              client_->addChangedItem(i.key());
                              client_->deleteETag(i.key());
                              client_->addETag(i.key(), i.value());
                          }
                      }
                      else{
                          qDebug() << "Item with eTag " + i.value().text() + "has been deleted\n\n";
                          client_->addDeletedItem(i.key());
                          client_->deleteETag(i.key());
                      }
                  }
                  for(i = mapTmp.begin(); i != mapTmp.end(); ++i){
                      if(!oldMap.contains(i.key())){
                          qDebug() << "There is a new Item with eTag: " + i.value().text() + "\n\n";
                          client_->addChangedItem(i.key());
                          client_->addETag(i.key(), i.value());
                      }
                  }

                  auto reply = client_->getChangedEvents();
                  connect(reply, &QNetworkReply::finished, [this, reply](){
                      QDomDocument res;
                      res.setContent(reply->readAll());
                      auto events = res.elementsByTagName("caldav:calendar-data");
                      auto href_list = res.elementsByTagName("D:href");
                      //QList<CalendarEvent> newEventstoShow;
                      for(int i=0; i<events.size(); i++){
                          qDebug() << events.at(i).toElement().text() + "\n\n";

                          /*
                          //salvo l'evento nella lista di eventi del calendario
                          QString el = events.at(i).toElement().text();
                          QTextStream stream(&el);
                          QPointer<Calendar> tmp = new Calendar("href_list.at(i).toElement().text()", stream);
                          if(calendar_.isNull())
                              calendar_ = tmp;
                          else
                              calendar_->events().append(tmp->events());
                          CalendarEvent newEvent(href_list.at(i).toElement().text(), stream, nullptr);
                          newEventstoShow.append(newEvent);


                          */
                      }
                      client_->clearChangedItems();
                      //setShowing_events(&newEventstoShow);
                  });

              });
          }
        });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
   refresh_calendar_events();
   QWidget::resizeEvent(event);
}

void MainWindow::updateTableToNDays(int n)
{
    ui->calendarTable->setColumnCount(n);
    QDate d = ui->calendarWidget->selectedDate();

    for(int i = 0; i < n; i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(d.addDays(i).toString("ddd\ndd"));
        ui->calendarTable->setHorizontalHeaderItem(i, item);
    }
    QList<CalendarEvent>* selected = new QList<CalendarEvent>;
    for(CalendarEvent& ev : calendar_->events())
    {
        auto recurs = ev.RecurrencesInRange(d.startOfDay(), d.addDays(n).endOfDay());
        for(const QDateTime& dt : recurs)
        {
            CalendarEvent new_ev = ev;
            auto diff = ev.getStartDateTime().time().msecsTo(ev.getEndDateTime().time());

            new_ev.setStartDateTime(dt);
            new_ev.setEndDateTime(dt.addMSecs(diff));
            selected->append(new_ev);
        }
    }
    setShowing_events(selected);
}


void MainWindow::on_actionOgni_10_secondi_triggered()
{
    qDebug() << "Tempo di sincronizzazione: 10 secondi";
    timer_->start(10000);
}
void MainWindow::on_actionOgni_30_secondi_triggered()
{
    qDebug() << "Tempo di sincronizzazione: 30 secondi";
    timer_->start(30000);
}
void MainWindow::on_actionOgni_minuto_triggered()
{
    qDebug() << "Tempo di sincronizzazione: 1 minuto";
    timer_->start(60000);
}
void MainWindow::on_actionOgni_10_minuti_triggered()
{
    qDebug() << "Tempo di sincronizzazione: 10 minuti";
    timer_->start(600000);
}
