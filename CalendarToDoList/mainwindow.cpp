#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QLabel>
#include <QMessageBox>

#include "./ui_mainwindow.h"
#include "CalendarClient/CalendarClient.h"
#include "widgets/calendartable.h"
#include "widgets/eventwidget.h"
#include "widgets/taskwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      calendar_(ICalendar()),
      timer_(new QTimer(this)),
      client_(new CalendarClient(this)),
      sync_token_supported_(false) {
  // Setup
  ui->setupUi(this);
  ui->calendarTable->init();
  qDebug() << "Starting...\n";

  // Internal signals
  connect(timer_, &QTimer::timeout, this,
          &MainWindow::on_actionSincronizza_triggered);
  // UI
  connect(ui->testButton, &QPushButton::clicked, this,
          &MainWindow::refresh_calendar_events);

  connect(ui->calendarWidget, &QCalendarWidget::clicked, [this](QDate date) {
    refresh_calendar_events();
    ui->calendarTable->setVisualMode(ui->calendarTable->visualMode(), date);
  });

  connect(ui->actionOgni_10_secondi, &QAction::triggered,
          [this]() { timer_->start(10000); });
  connect(ui->actionOgni_30_secondi, &QAction::triggered,
          [this]() { timer_->start(30000); });
  connect(ui->actionOgni_minuto, &QAction::triggered,
          [this]() { timer_->start(60000); });
  connect(ui->actionOgni_10_minuti, &QAction::triggered,
          [this]() { timer_->start(600000); });
  connect(ui->createEvent, &QPushButton::clicked,
          [this]() { on_request_editing_form(CalendarEvent()); });

  // Get allowed methods
  auto methods_reply = client_->findOutCalendarSupport();
  connect(methods_reply, &QNetworkReply::finished, [methods_reply, this]() {
    if (!methods_reply->hasRawHeader("Allow")) {
      qWarning("Cannot parse allowed methods");
      QMessageBox::critical(this, "Initialization error",
                            "The server do not send the allowed methods.");
      exit(-1);
    }
    for (QByteArray &method : methods_reply->rawHeader("Allow").split(','))
      client_->getSupportedMethods().insert(method.trimmed());

    auto props_reply = client_->findOutSupportedProperties();
    connect(props_reply, &QNetworkReply::finished, [props_reply, this]() {
      QDomDocument res;
      res.setContent(props_reply->readAll());
      QDomNodeList propstat = res.elementsByTagName("d:propstat");

      for (int i = 0; i < propstat.length(); i++) {
        QDomElement current = propstat.at(i).toElement();
        QString status =
            current.elementsByTagName("d:status").at(0).toElement().text();
        QDomNodeList sync_token = current.elementsByTagName("d:sync-token");
        QDomNodeList ctag = current.elementsByTagName("cs:getctag");
        if (status.contains("200") && sync_token.length() != 0) {
          sync_token_supported_ = true;
          client_->setSyncToken(sync_token.at(0).toElement().text());
        }
        if (status.contains("200") && ctag.length() != 0)
          client_->setCTag(ctag.at(0).toElement().text());
      }

      if (!sync_token_supported_) qWarning() << "Using cTag is deprecated";

      refresh_calendar_events();
      timer_->start(20000);
    });
  });
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::refresh_calendar_events() {
  QDate selected_date = ui->calendarWidget->selectedDate();
  QDate end_date = selected_date.addDays(ui->calendarTable->columnCount());

  ui->calendarTable->clearShowingWidgets();

  // prendo tutti gli eventi dalla data selezionata a una settimana dopo
  auto reply = client_->getDateRangeEvents(
      QDateTime(selected_date, QTime(0, 0)), QDateTime(end_date, QTime(0, 0)));
  connect(
      reply, &QNetworkReply::finished,
      [this, reply, selected_date, end_date]() {
        QDomDocument res;
        res.setContent(reply->readAll());
        QDomNodeList responses = res.elementsByTagName("d:response");

        for (int i = 0; i < responses.length(); i++) {
          QDomElement current = responses.at(i).toElement();
          QString calendar_data = current.elementsByTagName("cal:calendar-data")
                                      .at(0)
                                      .toElement()
                                      .text();
          QString href =
              current.elementsByTagName("d:href").at(0).toElement().text();
          QString eTag =
              current.elementsByTagName("d:getetag").at(0).toElement().text();

          QTextStream stream(&calendar_data);
          ICalendar tmp = ICalendar(href, eTag, stream);

          for (CalendarEvent &ev : tmp.events()) {
            CalendarTableItem<CalendarEvent> *widget =
                ui->calendarTable->createEventWidget(ev);
            if (widget != nullptr)
              connect(widget, &CalendarTableItem<CalendarEvent>::clicked,
                      [this, widget]() {
                        on_request_editing_form(widget->item(), true);
                      });
            // calendar_.events().append(ev);
          }
        }
      });

  // prendo tutti i task dalla data selezionata a una settimana dopo
  auto reply2 = client_->getDateRangeTasks(
      QDateTime(selected_date, QTime(0, 0)), QDateTime(end_date, QTime(0, 0)));
  connect(
      reply2, &QNetworkReply::finished,
      [this, reply2, selected_date, end_date]() {
        QDomDocument res;
        res.setContent(reply2->readAll());
        QDomNodeList responses = res.elementsByTagName("d:response");

        for (int i = 0; i < responses.length(); i++) {
          QDomElement current = responses.at(i).toElement();
          QString calendar_data = current.elementsByTagName("cal:calendar-data")
                                      .at(0)
                                      .toElement()
                                      .text();
          QString href =
              current.elementsByTagName("d:href").at(0).toElement().text();
          QString eTag =
              current.elementsByTagName("d:getetag").at(0).toElement().text();

          QTextStream stream(&calendar_data);
          ICalendar tmp = ICalendar(href, eTag, stream);

          for (Task &t : tmp.tasks()) {
            CalendarTableItem<Task> *widget =
                ui->calendarTable->createTaskWidget(t);
            if (widget != nullptr)
              connect(widget, &CalendarTableItem<CalendarEvent>::clicked,
                      [this, widget]() {
                        on_request_editing_form(widget->item(), true);
                      });
          }
        }
      });
}

void MainWindow::on_actionSincronizza_triggered() {
  if (!sync_token_supported_ && client_->getCTag().isEmpty()) return;

  if (!sync_token_supported_) {
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
        // TODO: ottimizzare e rendere funzionante
        auto reply1 = client_->lookForChanges();  // ottengo gli eTag per vedere
        // quali sono cambiati
        connect(reply1, &QNetworkReply::finished, [this, reply1]() {
          qDebug() << reply1->readAll();
          return;
          QHash<QString, QString> mapTmp;
          compareElements(*reply1, mapTmp);
          fetchChangedElements(mapTmp);
        });
      }
    });

  } else {
    auto reply2 = client_->receiveChangesBySyncToken();
    connect(reply2, &QNetworkReply::finished, [this, reply2]() {
      QDomDocument xml;
      xml.setContent(reply2->readAll());
      qDebug() << xml.toString();
      QDomNodeList responses = xml.elementsByTagName("d:response");
      QString sync_token =
          xml.elementsByTagName("d:sync-token").at(0).toElement().text();

      for (int i = 0; i < responses.length(); i++) {
        QDomElement current = responses.at(i).toElement();

        QString icalendar = current.elementsByTagName("cal:calendar-data")
                                .at(0)
                                .toElement()
                                .text();
        QString etag =
            current.elementsByTagName("d:getetag").at(0).toElement().text();
        QString href =
            current.elementsByTagName("d:href").at(0).toElement().text();
        QString status =
            current.elementsByTagName("d:status").at(0).toElement().text();

        client_->setSyncToken(sync_token);
        QTextStream stream(&icalendar);
        ICalendar cal = ICalendar(href, etag, stream);

        if (status.contains("200"))
          for (CalendarEvent &event : cal.events()) {
            CalendarTableItem<CalendarEvent> *widget =
                ui->calendarTable->createEventWidget(event);
            if (widget != nullptr)
              connect(widget, &CalendarTableItem<CalendarEvent>::clicked,
                      [this, widget]() {
                        on_request_editing_form(widget->item(), true);
                      });
          }
        if (status.contains("404")) ui->calendarTable->removeByHref(href);
      }
    });
  }
}

void MainWindow::compareElements(QNetworkReply &reply,
                                 QHash<QString, QString> &mapTmp) {
  // creo una mappa con gli href e gli etag nuovi
  QDomDocument res;
  res.setContent(reply.readAll());
  auto hrefs_list = res.elementsByTagName("D:href");
  auto eTags = res.elementsByTagName("D:getetag");

  if (eTags.isEmpty()) {
    qDebug() << "Calendar already up to date";
    return;
  }

  for (int i = 0; i < eTags.size(); i++) {
    mapTmp.insert(hrefs_list.at(i).toElement().text(),
                  eTags.at(i).toElement().text());
  }

  // confronto la nuova mappa con quella esistente
  // e aggiorno la lista di eTag nel client
  QSet<QString> processed;
  // Deleted and updated events
  for (CalendarEvent &ev : calendar_.events()) {
    QString href = ev.href();
    if (mapTmp.contains(href)) {
      if (mapTmp[href].isEmpty()) {
        qDebug() << "Item with href " + href + "has been deleted\n\n";
      } else {
        qDebug() << "Item with href " + href +
                        "has a new etag: " + mapTmp[href] + "\n\n";
        client_->deleteChangedItem(href);
        client_->addChangedItem(href);
      }
    } else {
      qDebug() << "Item with href " + href + "has been deleted\n\n";
    }
    calendar_.events().removeOne(ev);
    processed += href;
  }
  // Added events
  for (auto i = mapTmp.constBegin(); i != mapTmp.constEnd(); i++) {
    if (!processed.contains(i.key())) {
      qDebug() << "There is a new Item with eTag: " + i.value() + "\n\n";
      client_->addChangedItem(i.key());
    }
  }
}

void MainWindow::fetchChangedElements(QHash<QString, QString> &mapTmp) {
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
      ICalendar tmp = ICalendar(href_list.at(i).toElement().text(), "", stream);
      for (CalendarEvent &ev : tmp.events()) {
        QString hrefToSearch = ev.href();
        QString eTagToPut = mapTmp.find(hrefToSearch).value();
        ev.setETag(eTagToPut);
      }
      /*
      if (calendar_.isNull())
        calendar_ = tmp;
      else
        calendar_->events().append(tmp->events());
        */
    }
    client_->clearChangedItems();
  });
}

void MainWindow::on_request_editing_form(CalendarEvent event, bool isEvent) {
  bool existing = ui->calendarTable->getShowingEvents().contains(event.uid());
  if (dynamic_cast<Task *>(&event)) qDebug() << "Taks";
  CreateEventForm form(&event, *client_, calendar_, existing, this);
  int code = form.exec();
  CalendarEvent modified_event = form.getEvent();
  if (code == QDialog::Accepted)
    ui->calendarTable->createEventWidget(modified_event);
  else if (code == 2)
    ui->calendarTable->removeByHref(modified_event.href());
}
