#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QLabel>
#include <iostream>

#include "./ui_mainwindow.h"
#include "CalendarClient/CalendarClient.h"
#include "widgets/calendartable.h"
#include "widgets/eventwidget.h"
#include "widgets/taskwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      calendar_(new Calendar(this)),
      timer_(new QTimer(this)),
      showing_events_(QList<CalendarEvent>()),
      showing_tasks_(QList<Task>()),
      single_shot_timer_(new QTimer(this)) {
  ui->setupUi(this);
  ui->calendarTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->calendarTable->verticalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  single_shot_timer_->setSingleShot(true);
  qDebug() << "Starting...\n";

  // Force user to authenticate
  // if (auth_.isNull()) auth_ = new Authenticator(this);
  // QEventLoop loop;
  // connect(auth_->google, &QOAuth2AuthorizationCodeFlow::granted, &loop,
  //        &QEventLoop::quit);
  // loop.exec();

  /* TESTING CALENDAR TABLE
  CalendarTable *t = new CalendarTable();
  ui->horizontalLayout->addWidget(t);
  t->SetDays({"Monday", "Tuesday"});
  */

  // Internal signals
  connect(this, &MainWindow::show, this,
          &MainWindow::on_showing_events_changed);
  connect(this, &MainWindow::show, this, &MainWindow::on_showing_tasks_changed);
  connect(timer_, &QTimer::timeout, this,
          &MainWindow::on_actionSincronizza_triggered);
  connect(single_shot_timer_, &QTimer::timeout,
          [this]() { updateTableToNDays(ui->calendarTable->columnCount()); });
  // UI
  connect(ui->testButton, &QPushButton::clicked, this,
          &MainWindow::refresh_calendar_events);
  connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this,
          &MainWindow::refresh_calendar_events);
  connect(ui->actionOgni_10_secondi, &QAction::triggered,
          [this]() { timer_->start(10000); });
  connect(ui->actionOgni_30_secondi, &QAction::triggered,
          [this]() { timer_->start(30000); });
  connect(ui->actionOgni_minuto, &QAction::triggered,
          [this]() { timer_->start(60000); });
  connect(ui->actionOgni_10_minuti, &QAction::triggered,
          [this]() { timer_->start(600000); });
  connect(ui->createEvent, &QPushButton::clicked,
          [this]() { on_request_editing_form(); });

  client_ = new CalendarClient(this);

  /*auto reply = client_->findOutCalendarSupport();
  connect(reply, &QNetworkReply::finished, [reply](){
     auto list = reply->rawHeaderPairs();
     for(auto el : list){
         qDebug() << el.first + ":     " + el.second;
     }
  });*/

  auto reply = client_->findOutSupportedProperties();
    connect(reply, &QNetworkReply::finished, [reply](){

       qDebug() << reply->readAll();
   });

  // ottengo il cTag
  auto reply1 = client_->obtainCTag();
  connect(reply1, &QNetworkReply::finished, [this, reply1]() {
    QDomDocument res;
    res.setContent(reply1->readAll());
    auto lista = res.elementsByTagName("cs:getctag");
    client_->setCTag(lista.at(0).toElement().text());
  });

  // ottengo il sync-token
  auto reply2 = client_->requestSyncToken();
  connect(reply2, &QNetworkReply::finished, [this, reply2]() mutable {
    QDomDocument res;
    res.setContent(reply2->readAll());
    auto lista = res.elementsByTagName("d:sync-token");
    client_->setSyncToken(lista.at(0).toElement().text());

    reply2 = client_->receiveChangesBySyncToken();
    connect(reply2, &QNetworkReply::finished, [this, reply2]() mutable {
        //qDebug() << reply2->readAll();
    });
  });

  //ottengo tutti gli eventi nel calendario
  refresh_calendar_events();
}

MainWindow::~MainWindow() {
  showing_events_.clear();
  showing_tasks_.clear();
  delete ui;
}

void MainWindow::refresh_calendar_events() {
  auto reply = client_->getAllElements();
  connect(reply, &QNetworkReply::finished, [this, reply]() {
    calendar_->events().clear();

    QDomDocument res;
    res.setContent(reply->readAll());

    auto calendars = res.elementsByTagName("caldav:calendar-data");
    auto hrefs_list = res.elementsByTagName("D:href");
    auto eTags = res.elementsByTagName("D:getetag");

    for (int i = 0; i < calendars.size(); i++) {
      QString el = calendars.at(i).toElement().text();
      QString href = hrefs_list.at(i).toElement().text();
      QString eTag = eTags.at(i).toElement().text();
      QTextStream stream(&el);
      QPointer<Calendar> tmp = new Calendar(href, eTag, stream);

      QString line = stream.readLine();
      while (!line.isNull()) {
        if (line.contains("BEGIN:VEVENT")) {
          calendar_->events().append(tmp->events());
        } else {
          calendar_->tasks().append(tmp->tasks());
        }
      }
    }

    // salvo gli eTags per vedere i futuri cambiamenti
    //è una mappa di <href, eTag>
    for (int i = 0; i < eTags.size(); i++) {
      client_->addETag(hrefs_list.at(i).toElement().text(),
                       eTags.at(i).toElement().text());
    }

    updateTableToNDays(ui->calendarTable->columnCount());
  });
}

/*void MainWindow::on_seeIfChanged_clicked() {
  auto reply = client_->obtainCTag();
  connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
    QDomDocument q;
    q.setContent(reply->readAll());
    QDomElement thisCTag = q.elementsByTagName("cs:getctag").at(0).toElement();
    if (client_->getCTag() == thisCTag.text()) {
      client_->lookForChanges();
    }
  });
}*/

void MainWindow::on_showing_events_changed() {
  int column_width = ui->calendarTable->columnWidth(0);
  int row_heigth = ui->calendarTable->rowHeight(0);

  QDate selected_date = ui->calendarWidget->selectedDate();

  for (auto &event : showing_events_) {
    EventWidget *widget = new EventWidget(event, *client_, *calendar_,
                                          ui->calendarTable->viewport());
    QTime start_time = event.getStartDateTime().time();
    int days_long = event.getStartDateTime().daysTo(event.getEndDateTime());
    int time_long = event.getEndDateTime().time().hour() - start_time.hour();

    int x_pos =
        column_width * selected_date.daysTo(event.getStartDateTime().date());

    int y_pos = 0;
    for (int i = 0; i <= start_time.hour(); i++)
      y_pos += ui->calendarTable->rowHeight(i);
    y_pos += (start_time.minute() / 60.0) * ui->calendarTable->rowHeight(0);

    ui->calendarTable->scrollToTop();
    if (days_long == 0 && !event.all_day()) {
      widget->resize(column_width, row_heigth * time_long);
      widget->move(x_pos, y_pos);
    } else {
      widget->move(x_pos, 0);
      widget->resize((days_long + 1) * column_width, row_heigth);
    }

    // Connection to edit
    connect(widget, &EventWidget::clicked, [this, widget]() {
      on_request_editing_form(widget->event(), true);
    });
    widget->show();
  }
}

void MainWindow::on_showing_tasks_changed() {
  int column_width = ui->calendarTable->columnWidth(0);
  int row_heigth = ui->calendarTable->rowHeight(0);

  QDate selected_date = ui->calendarWidget->selectedDate();

  for (auto &task : showing_tasks_) {
    EventWidget *widget = new EventWidget(task, *client_, *calendar_,
                                          ui->calendarTable->viewport());
    QTime start_time = task.getStartDateTime().time();
    int days_long = task.getStartDateTime().daysTo(task.getEndDateTime());

    int x_pos =
        column_width * selected_date.daysTo(task.getStartDateTime().date());

    int y_pos = 0;
    for (int i = 0; i <= start_time.hour(); i++)
      y_pos += ui->calendarTable->rowHeight(i);
    y_pos += (start_time.minute() / 60.0) * ui->calendarTable->rowHeight(0);

    ui->calendarTable->scrollToTop();
    if (days_long == 0 && !task.all_day()) {
      widget->resize(column_width, row_heigth);
      widget->move(x_pos, y_pos);
    } else {
      widget->move(x_pos, 0);
      widget->resize((days_long + 1) * column_width, row_heigth);
    }

    // Connection to edit
    connect(widget, &EventWidget::clicked, [this, widget]() {
      on_request_editing_form(widget->event(), false);
    });
    widget->show();
  }
}

QList<CalendarEvent> MainWindow::showing_events() const {
  return showing_events_;
}

void MainWindow::setShowing_events(QList<CalendarEvent> newShowing_events) {
  // if (showing_events_ == newShowing_events) return;
  showing_events_ = newShowing_events;
}

void MainWindow::on_calendarWidget_clicked(const QDate &date) {
  QDate d(date);
  QTableWidgetItem *item;
  if (ui->calendarTable->columnCount() == 1) {
    item = new QTableWidgetItem(d.toString("ddd\ndd"));
    ui->calendarTable->setHorizontalHeaderItem(0, item);
  } else {
    for (int i = 0; i < ui->calendarTable->columnCount(); i++) {
      item = new QTableWidgetItem(d.addDays(i).toString("ddd\ndd"));
      ui->calendarTable->setHorizontalHeaderItem(i, item);
    }
  }
}

void MainWindow::on_actionSincronizza_triggered() {
  // ottengo il nuovo cTag e lo confronto con il vecchio
  auto reply = client_->obtainCTag();
  connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
    QDomDocument res;
    res.setContent(reply->readAll());
    QString newCTag =
        res.elementsByTagName("cs:getctag").at(0).toElement().text();

    if (newCTag == client_->getCTag()) {
      qDebug() << "Calendar already up to date";
    } else {  // se non sono uguali, qualcosa è cambiato
      client_->setCTag(newCTag);
      reply = client_->lookForChanges();  // ottengo gli eTag per vedere quali
                                          // sono cambiati
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        // creo una mappa con gli href e gli etag nuovi
        QDomDocument res;
        res.setContent(reply->readAll());
        auto hrefs_list = res.elementsByTagName("D:href");
        auto eTags = res.elementsByTagName("D:getetag");
        QHash<QString, QString> mapTmp;
        for (int i = 0; i < eTags.size(); i++) {
          mapTmp.insert(hrefs_list.at(i).toElement().text(),
                        eTags.at(i).toElement().text());
        }

        // confronto la nuova mappa con quella esistente
        // e aggiorno la lista di eTag nel client
        QSet<QString> processed;
        // Deleted and updated events
        for (CalendarEvent &ev : calendar_->events()) {
          QString href = ev.getHREF();
          if (mapTmp.contains(href)) {
            qDebug() << "Item with href " + href +
                            "has a new etag: " + mapTmp[href] + "\n\n";
            client_->deleteChangedItem(href);
            client_->addChangedItem(href);
          } else {
            qDebug() << "Item with href " + href + "has been deleted\n\n";
          }
          calendar_->events().removeOne(ev);
          processed += href;
        }
        // Added events
        for (auto i = mapTmp.constBegin(); i != mapTmp.constEnd(); i++) {
          if (!processed.contains(i.key())) {
            qDebug() << "There is a new Item with eTag: " + i.value() + "\n\n";
            client_->addChangedItem(i.key());
          }
        }

        auto reply = client_->getChangedElements();
        connect(reply, &QNetworkReply::finished, [this, reply, mapTmp]() {
          QDomDocument res;
          res.setContent(reply->readAll());
          auto events = res.elementsByTagName("caldav:calendar-data");
          auto href_list = res.elementsByTagName("D:href");
          for (int i = 0; i < events.size(); i++) {
            // qDebug() << events.at(i).toElement().text() + "\n\n";

            // salvo l'evento nella lista di eventi del calendario
            QString el = events.at(i).toElement().text();
            QTextStream stream(&el);
            QPointer<Calendar> tmp =
                new Calendar(href_list.at(i).toElement().text(), "", stream);
            for (CalendarEvent &ev : tmp->events()) {
              QString hrefToSearch = ev.getHREF();
              QString eTagToPut = mapTmp.find(hrefToSearch).value();
              ev.setETag(eTagToPut);
            }
            if (calendar_.isNull())
              calendar_ = tmp;
            else
              calendar_->events().append(tmp->events());
          }
          client_->clearChangedItems();
          updateTableToNDays(ui->calendarTable->columnCount());
        });
      });
    }
  });
}

void MainWindow::on_request_editing_form(CalendarEvent *event, bool isEvent) {
  bool existing = event != nullptr;
  if (!existing) event = new CalendarEvent;
  CreateEventForm form(event, *client_, *calendar_, existing, isEvent, this);
  connect(&form, &CreateEventForm::requestView,
          [this]() { updateTableToNDays(ui->calendarTable->columnCount()); });
  form.exec();
}

const QList<Task> &MainWindow::showing_tasks() const { return showing_tasks_; }

void MainWindow::setShowing_tasks(const QList<Task> &newShowing_tasks) {
  showing_tasks_ = newShowing_tasks;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  single_shot_timer_->start(500);
  updateTableToNDays(ui->calendarTable->columnCount());
}

void MainWindow::updateTableToNDays(int n) {
  ui->calendarTable->setColumnCount(n);
  QDate d = ui->calendarWidget->selectedDate();

  for (int i = 0; i < n; i++) {
    QTableWidgetItem *item =
        new QTableWidgetItem(d.addDays(i).toString("ddd\ndd"));
    ui->calendarTable->setHorizontalHeaderItem(i, item);
  }
  // EVENTS
  QList<CalendarEvent> selected;
  for (CalendarEvent &ev : calendar_->events()) {
    auto recurs =
        ev.RecurrencesInRange(d.startOfDay(), d.addDays(n).endOfDay());
    for (const QDateTime &dt : recurs) {
      CalendarEvent new_ev = ev;
      auto diff =
          ev.getStartDateTime().time().msecsTo(ev.getEndDateTime().time());

      new_ev.setStartDateTime(dt);
      new_ev.setEndDateTime(dt.addMSecs(diff));
      selected += new_ev;
    }
  }
  setShowing_events(selected);
  // TASKS
  QList<Task> selected_tasks;
  for (Task &t : calendar_->tasks()) {
    if (t.getEndDateTime() >= d.startOfDay() &&
        t.getEndDateTime() <= d.addDays(n).endOfDay()) {
      selected_tasks += t;
    }
  }
  setShowing_tasks(selected_tasks);

  // Delete all previous widgets
  for (auto child : ui->calendarTable->viewport()->children())
    if (qobject_cast<EventWidget *>(child) != nullptr) child->deleteLater();
  // Show the new widgets
  emit show();
}
