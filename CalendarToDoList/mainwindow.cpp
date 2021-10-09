#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QLabel>
#include <QMessageBox>
#include <functional>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "./ui_mainwindow.h"
#include "CalendarClient/CalendarClient.h"
#include "widgets/calendartable.h"
#include "widgets/calendartableitem.h"
#include "widgets/userform.h"
#include "widgets/usercalendarschoice.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      calendar_(ICalendar()),
      timer_(new QTimer(this)),
      client_(new CalendarClient(this)),
      sync_token_supported_(false),
      readyUser_(false),
      readyEvent(false) {


    // read auth file or create it from user given data
    QFile auth_file("auth.json");
    if (!auth_file.exists()) {
        Userform form(*client_);
        form.exec();
    } else {
        auth_file.open(QFile::OpenModeFlag::ReadOnly);
        QJsonObject json = QJsonDocument().fromJson(auth_file.readAll()).object();

        QUrl principal = QUrl(json["principal"].toString());
        client_->setPrincipal(principal);
        QUrl host = QUrl(json["host"].toString());
        client_->setHost(host);
        QByteArray credentials = (json["username"].toString() + ":" + json["password"].toString()).toUtf8().toBase64();
        client_->setCredentials(credentials);
    }

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

    connect(ui->actionOgni_10_secondi, &QAction::triggered, [this]() {
        if (!readyEvent)
            QMessageBox::warning(
                        this, "Attendi ancora un pò",
                        "Il server non è ancora pronto, riprova fra qualche istante");
        else
            timer_->start(10000);
    });
    connect(ui->actionOgni_30_secondi, &QAction::triggered, [this]() {
        if (!readyEvent)
            QMessageBox::warning(
                        this, "Attendi ancora un pò",
                        "Il server non è ancora pronto, riprova fra qualche istante");
        else
            timer_->start(30000);
    });
    connect(ui->actionOgni_minuto, &QAction::triggered, [this]() {
        if (!readyEvent)
            QMessageBox::warning(
                        this, "Attendi ancora un pò",
                        "Il server non ha ancora risposto, riprova fra qualche istante");
        else
            timer_->start(60000);
    });
    connect(ui->actionOgni_10_minuti, &QAction::triggered, [this]() {
        if (!readyEvent)
            QMessageBox::warning(
                        this, "Attendi ancora un pò",
                        "Il server non è ancora pronto, riprova fra qualche istante");
        else
            timer_->start(600000);
    });

    connect(ui->createEvent, &QPushButton::clicked, [this]() {
        if (!readyEvent) {
            QMessageBox::warning(
                        this, "Attendi ancora un pò",
                        "Il server non è ancora pronto, riprova fra qualche istante");
        } else {
            showEditForm(ICalendarComponent());
        }
    });


    // get principal if user didn't specify it
    if(client_->getPrincipal().isEmpty()){
        tryGetPrincipal();
    }
    else{
        getUserCalendars();
    }
}

void MainWindow::tryGetPrincipal(){

    auto reply = client_->discoverUser();
    connect(reply, &QNetworkReply::finished, [reply, this]() {

        if (reply->error() != QNetworkReply::NoError || reply == nullptr) {
            qWarning("Non riesco a ottenere il link all'utente principale");
            QMessageBox::critical(
                        this, "Errore di inizializzazione",
                        "Il server non riesce a mandare il link all'utente principale");
            exit(-1);
        }

        QDomDocument res;
        res.setContent(reply->readAll());

        QString status = res.elementsByTagName("d:status").at(0).toElement().text();
        if (!status.contains("200")) {
            qWarning("Non riesco a ottenere il link all'utente principale");
            QMessageBox::critical(
                        this, "Errore di inizializzazione",
                        "Il server non riesce a mandare il link all'utente principale");
            exit(-1);
        }

        QUrl principal = res.elementsByTagName("d:href").at(1).toElement().text();
        if(!principal.isEmpty()){
            client_->setPrincipal(principal);

            QFile file("auth.json");
            file.open(QFile::OpenModeFlag::ReadWrite);
            QJsonObject json = QJsonDocument().fromJson(file.readAll()).object();
            json.remove("principal");
            json.insert("principal", principal.toString());
            QJsonDocument doc(json);
            file.resize(0);
            file.write(doc.toJson());
        }
        else{
            QMessageBox::critical(this, "Errore", "Necessario inserire l'url dell'utente principale");
            exit(-1);
        }
        getUserCalendars();
    });

}

void MainWindow::getUserCalendars() {

    // Get the link to all the principal user's calendars
    auto reply1 = client_->discoverUserCalendars();
    connect(reply1, &QNetworkReply::finished, [reply1, this]() {

        if (reply1->error() != QNetworkReply::NoError || reply1 == nullptr) {
            qWarning("Non riesco a ottenere il link ai calendari utente");
            QMessageBox::critical(
                        this, "Errore di inizializzazione",
                        "Il server non riesce a mandare il link ai calendari utente");
            exit(-1);
        }

        QDomDocument res;
        res.setContent(reply1->readAll());

        QString status = res.elementsByTagName("d:status").at(0).toElement().text();
        if (!status.contains("200")) {
            qWarning("Non riesco a ottenere il link ai calendari utente");
            QMessageBox::critical(
                        this, "Errore di inizializzazione",
                        "Il server non riesce a mandare il link ai calendari utente");
            exit(-1);
        }

        QString complete = client_->getHost().toString() + res.elementsByTagName("d:href").at(1).toElement().text();
        QUrl calendars = QUrl(complete);
        client_->setUserCalendars(calendars);

        //get the link to every specific principal user's calendar
        auto reply2 = client_->listUserCalendars();
        connect(reply2, &QNetworkReply::finished, [reply2, this]() {

            if (reply2->error() != QNetworkReply::NoError || reply2 == nullptr) {
                qWarning("Non riesco a ottenere i calendari utente");
                QMessageBox::critical(
                            this, "Errore di inizializzazione",
                            "Il server non riesce a mandare i calendari utente");
                exit(-2);
            }

            QDomDocument res;
            res.setContent(reply2->readAll());

            auto statusesList = res.elementsByTagName("d:status");
            for (int i = 0; i < statusesList.length(); i++) {
                if (!statusesList.at(i).toElement().text().contains("200")) {
                    qWarning("Non riesco a ottenere il link ai calendari utente");
                    QMessageBox::critical(
                                this, "Errore di inizializzazione",
                                "Il server non riesce a mandare il link ai calendari utente");
                    exit(-2);
                }
            }

            auto hrefList = res.elementsByTagName("d:href"); //first href must be ignored
            auto calendarNames = res.elementsByTagName("d:displayname");

            // ensure that it contains at least a calendar element in the CalDAV namespace
            auto cal1 = res.elementsByTagName("cal:calendar");
            auto cal2 = res.elementsByTagName("c:calendar");
            if(cal1.length()!=calendarNames.length() && cal2.length()!=calendarNames.length()){
                QMessageBox::critical(this, "Errore", "Calendario non supportato");
                exit(-1);
            }

            //ensure that both VEVENT and VTODO are supported
            /*auto calComp1 = res.elementsByTagName("cal:comp");
            for(int i=0; i<calComp1.length(); i++){
                if(!calComp1.at(i).toElement().attribute("name").contains("VEVENT")){
                    QMessageBox::critical(this, "Errore", "Un calendario non supporta gli eventi");
                    exit(-1);
                }
                if(!calComp1.at(i).toElement().attribute("name").contains("VTODO")){
                    QMessageBox::critical(this, "Errore", "Un calendario non supporta le attività");
                    exit(-1);
                }
            }
            auto calComp2 = res.elementsByTagName("c:comp");
            for(int i=0; i<calComp2.length(); i++){
                if(!calComp2.at(i).toElement().attribute("name").contains("VEVENT")){
                    QMessageBox::critical(this, "Errore", "Un calendario non supporta gli eventi");
                    exit(-1);
                }
                if(!calComp2.at(i).toElement().attribute("name").contains("VTODO")){
                    QMessageBox::critical(this, "Errore", "Un calendario non supporta le attività");
                    exit(-1);
                }
            }*/

            //get calendar name and relative href
            for(int i=0; i<calendarNames.length(); i++){
                QString url = hrefList.at(i+1).toElement().text();
                QString complete = client_->getHost().toString() + url;
                QUrl endpoint = QUrl(complete);
                client_->getUserCalendarsList().insert(calendarNames.at(i).toElement().text(), endpoint);
            }

            UserCalendarsChoice form(*client_);
            form.exec();
            initialize();
        });
    });
}

void MainWindow::initialize(){

    // Get allowed methods
    auto methods_reply = client_->findOutCalendarSupport();
    connect(methods_reply, &QNetworkReply::finished, [methods_reply, this]() {
        if (methods_reply->error() != QNetworkReply::NoError ||
                methods_reply == nullptr || !methods_reply->hasRawHeader("Allow") ||
                !methods_reply->hasRawHeader("allow")) {
            qWarning("Non riesco a ottenere i metodi supportati dal server");
            QMessageBox::critical(
                        this, "Errore di inizializzazione",
                        "Il server non riesce a mandare i metodi supportati");
            exit(-1);
        }
        if (methods_reply->hasRawHeader("Allow")) {
            for (QByteArray &method : methods_reply->rawHeader("Allow").split(',')) {
                client_->getSupportedMethods().insert(method.trimmed());
            }
        } else if (methods_reply->hasRawHeader("allow")) {
            for (QByteArray &method : methods_reply->rawHeader("allow").split(',')) {
                client_->getSupportedMethods().insert(method.trimmed());
            }
        }

        // Get supported properties
        auto props_reply = client_->findOutSupportedProperties();
        connect(props_reply, &QNetworkReply::finished, [props_reply, this]() {
            if (props_reply->error() != QNetworkReply::NoError ||
                    props_reply == nullptr) {
                qWarning("Non riesco a ottenere le proprietà supportate dal server");
                QMessageBox::critical(
                            this, "Errore di inizializzazione",
                            "Il server non riesce a mandare le proprietà supportate");
                exit(-2);
            }

            QDomDocument res;
            res.setContent(props_reply->readAll());
            QDomNodeList propstat = res.elementsByTagName("d:propstat");

            for (int i = 0; i < propstat.length(); i++) {
                QDomElement current = propstat.at(i).toElement();
                QString status =
                        current.elementsByTagName("d:status").at(0).toElement().text();
                if (!status.contains("200")) {
                    qWarning("Non riesco a ottenere le proprietà supportate dal server");
                    QMessageBox::critical(
                                this, "Errore di inizializzazione",
                                "Il server non riesce a mandare le proprietà supportate");
                    exit(-2);
                }
                QDomNodeList sync_token = current.elementsByTagName("d:sync-token");
                QDomNodeList ctag = current.elementsByTagName("cs:getctag");
                if (sync_token.length() != 0) {
                    sync_token_supported_ = true;
                    client_->setSyncToken(sync_token.at(0).toElement().text());
                }
                if (ctag.length() != 0) client_->setCTag(ctag.at(0).toElement().text());
            }

            if (!sync_token_supported_) qWarning() << "Using cTag is deprecated";


            refresh_calendar_events();
            timer_->start(20000);
        });
    });

}

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
        if (reply->error() != QNetworkReply::NoError || reply == nullptr) {
            qWarning("Non riesco a ottenere gli eventi dal server");
            QMessageBox::critical(this, "Errore di inizializzazione",
                                  "Il server non riesce a mandare gli eventi");
            exit(-3);
        }

        QDomDocument res;
        res.setContent(reply->readAll());

        auto statusesList = res.elementsByTagName("d:status");
        for (int i = 0; i < statusesList.length(); i++) {
            if (!statusesList.at(i).toElement().text().contains("200")) {
                qWarning("Non riesco a ottenere gli eventi dal server");
                QMessageBox::critical(this, "Errore di inizializzazione",
                                      "Il server non riesce a mandare gli eventi");
                exit(-3);
            }
        }

        QDomNodeList responses = res.elementsByTagName("d:response");
        qDebug() << "Parsing events...";

        for (int i = 0; i < responses.length(); i++) {
            QDomElement current = responses.at(i).toElement();
            ICalendar tmp = ICalendar().fromXmlResponse(current);

            for (ICalendarComponent &ev : tmp.components()) {
                if (!ev.getProperty("RRULE")) {
                    ui->calendarTable->createTableItem(ev, this);
                } else {
                    getExpansion(std::move(ev));
                }
            }
        }
        readyEvent = true;
    });
}

void MainWindow::on_actionSincronizza_triggered() {
    if (!readyEvent) {
        QMessageBox::warning(
                    this, "Attendi ancora un pò",
                    "Il server non è ancora pronto, riprova fra qualche istante");
        return;
    }
    if (!sync_token_supported_ && client_->getCTag().isEmpty()) return;
    qDebug() << "Syncing...";

    if (!sync_token_supported_) {
        // ottengo il nuovo cTag e lo confronto con il vecchio
        auto reply = client_->obtainCTag();
        connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
            if (reply->error() != QNetworkReply::NoError || reply == nullptr) {
                qWarning("Non riesco a ottenere il ctag dal server");
                QMessageBox::critical(this, "Errore di sincronizzazione",
                                      "Il server non riesce a mandare il ctag");
                return;
            }

            QDomDocument res;
            res.setContent(reply->readAll());

            QString status =
                    res.elementsByTagName("d:status").at(0).toElement().text();
            if (!status.contains("200")) {
                qWarning("Non riesco a ottenere il ctag dal server");
                QMessageBox::critical(this, "Errore di sincronizzazione",
                                      "Il server non riesce a mandare il ctag");
                return;
            }

            QString newCTag =
                    res.elementsByTagName("cs:getctag").at(0).toElement().text();
            if (newCTag == client_->getCTag()) {
                qDebug() << "Calendar already up to date";
            } else {  // se non sono uguali, qualcosa è cambiato
                client_->setCTag(newCTag);
                refresh_calendar_events();
            }
        });

    } else {
        auto reply2 = client_->receiveChangesBySyncToken();
        connect(reply2, &QNetworkReply::finished, [this, reply2]() {
            if (reply2->error() != QNetworkReply::NoError || reply2 == nullptr) {
                qWarning(
                            "Non riesco a ottenere eventuali cambiamenti a eventi o attività "
            "dal server");
                QMessageBox::critical(this, "Errore di sincronizzazione",
                                      "Il server non riesce a mandare eventuali "
                              "cambiamenti a eventi o attività");
            }

            QDomDocument xml;
            xml.setContent(reply2->readAll());

            auto statusesList = xml.elementsByTagName("d:status");
            for (int i = 0; i < statusesList.length(); i++) {
                if (!statusesList.at(i).toElement().text().contains("200") &&
                        !statusesList.at(i).toElement().text().contains("404")) {
                    qWarning(
                                "Non riesco a ottenere eventuali cambiamenti a eventi o attività "
              "dal server");
                    QMessageBox::critical(this, "Errore di inizializzazione",
                                          "Il server non riesce a mandare eventuali "
                                "cambiamenti a eventi o attività");
                }
            }

            QDomNodeList responses = xml.elementsByTagName("d:response");
            QString sync_token =
                    xml.elementsByTagName("d:sync-token").at(0).toElement().text();
            client_->setSyncToken(sync_token);

            for (int i = 0; i < responses.length(); i++) {
                QDomElement current = responses.at(i).toElement();
                QString status =
                        current.elementsByTagName("d:status").at(0).toElement().text();

                if (status.contains("200")) {
                    ICalendar cal = ICalendar().fromXmlResponse(current);
                    for (ICalendarComponent &event : cal.components()) {
                        if (!event.getProperty("RRULE")) {
                            ui->calendarTable->createTableItem(event, this);
                        } else {
                            qDebug() << "Syncing recurrent...";
                            getExpansion(std::move(event));
                        }
                    }
                }
                if (status.contains("404")) {
                    QString href =
                            xml.elementsByTagName("d:href").at(0).toElement().text();
                    ui->calendarTable->removeByHref(href);
                };
            }
        });
    }
}

void MainWindow::on_actionCambia_utente_server_triggered()
{
    Userform form(*client_);
    form.exec();
    if(client_->getPrincipal().isEmpty()){
        tryGetPrincipal();
    }
    else{
        getUserCalendars();
    }
}

void MainWindow::on_actionCambia_calendario_triggered()
{
    if(client_->getUserCalendarsList().isEmpty()){
        QMessageBox::warning(
                    this, "Attendi ancora un pò",
                    "Il server non è ancora pronto, riprova fra qualche istante");
    }
    else {
        UserCalendarsChoice form(*client_);
        form.exec();
        initialize();
    }
}

void MainWindow::getExpansion(ICalendarComponent &&original) {
    auto reply = client_->getExpandedRecurrentEvent(
                original.getUID(), ui->calendarTable->getDateTimeRange());
    connect(reply, &QNetworkReply::finished, this,
            [moved = std::move(original), reply, this]() {
        QDomDocument res;
        res.setContent(reply->readAll());
        qDebug() << res.toString();

        QDomNodeList responses = res.elementsByTagName("d:response");

        for (int i = 0; i < responses.length(); i++) {
            QDomElement current = responses.at(i).toElement();
            ICalendar tmp = ICalendar().fromXmlResponse(current);

            for (ICalendarComponent &ev : tmp.components()) {
                ICalendarComponent copy(moved);
                if (ev.getStartDateTime())
                    copy.setStartDateTime(ev.getStartDateTime().value());
                if (ev.getEndDateTime())
                    copy.setEndDateTime(ev.getEndDateTime().value());
                ui->calendarTable->createTableItem(copy, this);
            }
        }
    });
}

void MainWindow::showEditForm(ICalendarComponent component) {
    bool existing =
            ui->calendarTable->getShowingEvents().contains(component.getUID());
    CreateEventForm form(&component, *client_, calendar_, existing, this);
    int code = form.exec();
    if (code != QDialog::Rejected) on_actionSincronizza_triggered();
}

MainWindow::~MainWindow() { delete ui; }

