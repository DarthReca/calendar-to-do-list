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
      showing_tasks_(QList<Task>()) {
    ui->setupUi(this);

    ui->calendarTable->init();
    qDebug() << "Starting...\n";

    client_ = new CalendarClient(this);

    // Internal signals
    // connect(this, &MainWindow::show, this,
    //        &MainWindow::on_showing_events_changed);
    // connect(this, &MainWindow::show, this,
    // &MainWindow::on_showing_tasks_changed);
    connect(timer_, &QTimer::timeout, this,
            &MainWindow::on_actionSincronizza_triggered);
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

    // Chiedo quali sono i metodi supportati dal server
    auto reply = client_->findOutCalendarSupport();
    connect(reply, &QNetworkReply::finished, [reply, this]() {
        auto list = reply->rawHeaderPairs();
        for (auto &el : list) {
            if (el.first.toLower() == "allow") {
                for (auto &method : el.second.split(',')) {
                    client_->getSupportedMethods().insert(QString(method.trimmed()));
                }
                break;
            }
        }

        // ottengo il cTag
        auto reply1 = client_->findOutSupportedProperties();
        connect(reply1, &QNetworkReply::finished, [reply1, this]() {
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto lista1 = res.elementsByTagName("cs:getctag");
            if (lista1.at(0).toElement().text().isEmpty()) {
                auto reply1 = client_->obtainCTag();
                connect(reply1, &QNetworkReply::finished, [this, reply1]() {
                    QDomDocument res;
                    res.setContent(reply1->readAll());
                    auto lista = res.elementsByTagName("cs:getctag");
                    client_->setCTag(lista.at(0).toElement().text());
                });
            } else {
                client_->setCTag(lista1.at(0).toElement().text());
            }

            // ottengo il sync-token
            auto reply2 = client_->requestSyncToken();
            connect(reply2, &QNetworkReply::finished, [this, reply2]() mutable {
                QDomDocument res;
                res.setContent(reply2->readAll());
                auto lista = res.elementsByTagName("d:sync-token");
                client_->setSyncToken(lista.at(0).toElement().text());

                refresh_calendar_events();
            });
        });
    });
}

MainWindow::~MainWindow() {
    showing_events_.clear();
    showing_tasks_.clear();
    delete ui;
}

void MainWindow::refresh_calendar_events() {
    QDate selected_date = ui->calendarWidget->selectedDate();
    QDate end_date = selected_date.addDays(ui->calendarTable->columnCount());
    auto reply = /*client_->getDateRangeEvents(
              QDateTime(selected_date, QTime(0, 0)), QDateTime(end_date, QTime(0, 0)));
          */ client_->getAllElements();
    ui->calendarTable->clearShowingWidgets();
    connect(reply, &QNetworkReply::finished,
            [this, reply, selected_date, end_date]() {
        calendar_->events().clear();

        QDomDocument res;
        res.setContent(reply->readAll());

        auto calendars = res.elementsByTagName("cal:calendar-data");
        auto hrefs_list = res.elementsByTagName("d:href");
        auto eTags = res.elementsByTagName("d:getetag");

        for (int i = 0; i < calendars.size(); i++) {
            QString icalendar = calendars.at(i).toElement().text();
            QString href = hrefs_list.at(i).toElement().text();
            QString eTag = eTags.at(i).toElement().text();
            QTextStream stream(&icalendar);
            QPointer<Calendar> tmp = new Calendar(href, eTag, stream);

            for (CalendarEvent &ev : tmp->events()) {
                auto recurrences =
                        ev.recurrencesInRange(QDateTime(selected_date, QTime(0, 0)),
                                              QDateTime(end_date, QTime(0, 0)));
                for (QDateTime &rec : recurrences) {
                    CalendarEvent *new_event = new CalendarEvent(ev);
                    auto diff = ev.startDateTime().time().msecsTo(
                                ev.endDateTime().time());
                    new_event->setStartDateTime(rec);
                    new_event->setEndDateTime(rec.addMSecs(diff));
                    calendar_->events().append(*new_event);
                    EventWidget &widget =
                            ui->calendarTable->createEventWidget(*new_event);
                    connect(&widget, &EventWidget::clicked, [this, &widget]() {
                        on_request_editing_form(&widget.event(), true);
                    });
                }
            }
            for (Task &t : tmp->tasks()) {
                Task *new_task = new Task(t);
                calendar_->tasks().append(*new_task);
                EventWidget &widget =
                        ui->calendarTable->createEventWidget(*new_task);
                connect(&widget, &EventWidget::clicked, [this, &widget]() {
                    on_request_editing_form(&widget.event(), true);
                });
            }
        }

        // salvo gli eTags per vedere i futuri cambiamenti
        //è una mappa di <href, eTag>
        for (int i = 0; i < eTags.size(); i++) {
            client_->addETag(hrefs_list.at(i).toElement().text(),
                             eTags.at(i).toElement().text());
        }

        /*qDebug() << "UID: " + calendar_->events().at(0).uid() + "     " + calendar_->events().at(0).summary();
            auto reply3 = client_->getElementByUID(calendar_->events().at(0).uid());
            connect(reply3, &QNetworkReply::finished, [reply3, this]() {
                qDebug() << "Elemento per UID:" +reply3->readAll();
            });*/

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

// KEPT FOR COMPATIBILITY
void MainWindow::on_showing_events_changed() {}

// KEPT FOR COMPATIBILITY
void MainWindow::on_showing_tasks_changed() {}

QList<CalendarEvent> MainWindow::showing_events() const {
    return showing_events_;
}

void MainWindow::setShowing_events(QList<CalendarEvent> newShowing_events) {
    showing_events_ = newShowing_events;
}

void MainWindow::on_calendarWidget_clicked(const QDate &date) {
    ui->calendarTable->setVisualMode(ui->calendarTable->visualMode(),
                                     QDateTime(date, QTime::currentTime()));
}

void MainWindow::on_actionSincronizza_triggered() {
    if (client_->getSyncToken().isEmpty() && client_->getCTag().isEmpty()) return;

    if (client_->getSyncToken().isEmpty()) {
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
                auto reply1 = client_->lookForChanges();  // ottengo gli eTag per vedere
                // quali sono cambiati
                connect(reply1, &QNetworkReply::finished, [this, reply1]() {
                    QHash<QString, QString> mapTmp;
                    compareElements(*reply1, mapTmp);
                    fetchChangedElements(mapTmp);
                });
            }
        });

    } else {
        auto reply2 = client_->receiveChangesBySyncToken();
        connect(reply2, &QNetworkReply::finished, [this, reply2]() {
            QHash<QString, QString> mapTmp;
            compareElements(*reply2, mapTmp);
            if (!mapTmp.isEmpty()) {
                fetchChangedElements(mapTmp);
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
    for (CalendarEvent &ev : calendar_->events()) {
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
            QPointer<Calendar> tmp =
                    new Calendar(href_list.at(i).toElement().text(), "", stream);
            for (CalendarEvent &ev : tmp->events()) {
                QString hrefToSearch = ev.href();
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
}

void MainWindow::on_request_editing_form(CalendarEvent *event, bool isEvent) {
    bool existing = event != nullptr;
    if (!existing) event = new CalendarEvent;
    CreateEventForm form(event, *client_, *calendar_, existing, isEvent, this);
    // connect(&form, &CreateEventForm::requestView,
    //       [this]() { ui->calendarTable->createEventWidget(event); });
    form.exec();
    if (!existing) ui->calendarTable->createEventWidget(*event);
}

const QList<Task> &MainWindow::showing_tasks() const { return showing_tasks_; }

void MainWindow::setShowing_tasks(const QList<Task> &newShowing_tasks) {
    showing_tasks_ = newShowing_tasks;
}

// KEPT FOR COMPATIBILITY
void MainWindow::updateTableToNDays(int n) {}
