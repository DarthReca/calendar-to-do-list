#include "mainwindow.h"

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <functional>

#include "./ui_mainwindow.h"
#include "calendarclient.h"
#include "errormanager.h"
#include "widgets/calendartable.h"
#include "widgets/calendartableitem.h"
#include "widgets/usercalendarschoice.h"
#include "widgets/userform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      timer_(new QTimer(this)),
      client_(new CalendarClient()),
      sync_token_supported_(false),
      ready_user_(false),
      ready_event_(false)
      {
  // read auth file or create it from user given data
  QFile auth_file("auth.json");
  if (!auth_file.exists()) {
    Userform form(*client_);
    form.exec();
  } else {
    auth_file.open(QFile::OpenModeFlag::ReadOnly);
    QJsonObject json = QJsonDocument().fromJson(auth_file.readAll()).object();

    if (!json.contains("principal") || !json.contains("host") ||
        !json.contains("password") || !json.contains("username")) {
      QMessageBox::critical(
          this, "Chiavi mancanti in json",
          "auth.json deve contenere: host, principal, username e password");
      exit(INITIALIZATION_ERROR);
    }


    QUrl principal = QUrl(json["principal"].toString());
    QUrl host = QUrl(json["host"].toString());
    QByteArray credentials =
        (json["username"].toString() + ":" + json["password"].toString())
            .toUtf8()
            .toBase64();

    client_->setPrincipal(principal);
    client_->setHost(host);
    client_->setCredentials(credentials);
  }

  // Setup
  ui->setupUi(this);
  ui->calendarTable->init();
  qDebug() << "Starting...\n";

  // Internal signals
  connect(timer_, &QTimer::timeout, this, &MainWindow::synchronize);
  // UI

  connect(ui->calendarWidget, &QCalendarWidget::clicked, [this](QDate date) {
    refreshCalendarEvents();
    ui->calendarTable->setVisualMode(ui->calendarTable->visualMode(), date);
  });
  connect(ui->taskList, &ComponentListWidget::itemClicked, this,
          [this](QListWidgetItem *item) {
            showEditForm(ui->taskList->componentByItem(item));
          });
  connect(ui->calendarTable, &CalendarTable::calendarItemClicked, this,
          [this](CalendarTableItem *item) { showEditForm(item->component()); });

  // ACTIONS
  connect(ui->actionMostra_task_senza_data, &QAction::triggered,
          [this]() { ui->taskList->setVisible(!ui->taskList->isVisible()); });
  connect(ui->actionSincronizza, &QAction::triggered, this,
          &MainWindow::synchronize);
  connect(ui->actionCambia_utente_server, &QAction::triggered, this,
          &MainWindow::changeUserServer);
  connect(ui->actionCambia_calendario, &QAction::triggered, this,
          &MainWindow::changeCalendar);

  connect(ui->actionOgni_10_secondi, &QAction::triggered, [this]() {
    if (!ready_event_)
      QMessageBox::warning(
          this, "Attendi ancora un pò",
          "Il server non è ancora pronto, riprova fra qualche istante");
    else
      timer_->start(10000);
  });
  connect(ui->actionOgni_30_secondi, &QAction::triggered, [this]() {
    if (!ready_event_)
      QMessageBox::warning(
          this, "Attendi ancora un pò",
          "Il server non è ancora pronto, riprova fra qualche istante");
    else
      timer_->start(30000);
  });
  connect(ui->actionOgni_minuto, &QAction::triggered, [this]() {
    if (!ready_event_)
      QMessageBox::warning(
          this, "Attendi ancora un pò",
          "Il server non ha ancora risposto, riprova fra qualche istante");
    else
      timer_->start(60000);
  });
  connect(ui->actionOgni_10_minuti, &QAction::triggered, [this]() {
    if (!ready_event_)
      QMessageBox::warning(
          this, "Attendi ancora un pò",
          "Il server non è ancora pronto, riprova fra qualche istante");
    else
      timer_->start(600000);
  });

  connect(ui->createEvent, &QPushButton::clicked, [this]() {
    if (!ready_event_) {
      QMessageBox::warning(
          this, "Attendi ancora un pò",
          "Il server non è ancora pronto, riprova fra qualche istante");
    } else {
      showEditForm(ICalendarComponent());
    }
  });

  // get principal if user didn't specify it
  if (client_->getPrincipal().isEmpty()) {
    tryGetPrincipal();
  } else {
    getUserCalendars();
  }
}

void MainWindow::tryGetPrincipal() {
  auto reply = client_->discoverUser();
  connect(reply, &QNetworkReply::finished, [reply, this]() {
    if (reply->error() != QNetworkReply::NoError) {
      qWarning("Non riesco a ottenere il link all'utente principale");
      ErrorManager::initializationError(
          this, "Il server non riesce a mandare il link all'utente principale");
    }

    QDomDocument res;
    res.setContent(reply->readAll());

    QString status = res.elementsByTagName("d:status").at(0).toElement().text();
    if (!status.contains("200")) {
      qWarning("Non riesco a ottenere il link all'utente principale");
      ErrorManager::initializationError(
          this, "Il server non riesce a mandare il link all'utente principale");
    }

    QUrl principal = res.elementsByTagName("d:href").at(1).toElement().text();
    if (!principal.isEmpty()) {
      client_->setPrincipal(principal);

      QFile file("auth.json");
      file.open(QFile::OpenModeFlag::ReadWrite);
      QJsonObject json = QJsonDocument().fromJson(file.readAll()).object();
      json.remove("principal");
      json.insert("principal", principal.toString());
      QJsonDocument doc(json);
      file.resize(0);
      file.write(doc.toJson());
    } else {
      QMessageBox::critical(this, "Errore",
                            "Necessario inserire l'url dell'utente principale");
      exit(LOGIC_ERROR);
    }
    getUserCalendars();
  });
}

void MainWindow::getUserCalendars() {
  // Get the link to all the principal user's calendars
  auto reply1 = client_->discoverUserCalendars();
  connect(reply1, &QNetworkReply::finished, [reply1, this]() {
    if (reply1->error() != QNetworkReply::NoError) {
      qWarning("Non riesco a ottenere il link ai calendari utente");
      ErrorManager::initializationError(
          this, "Il server non riesce a mandare il link ai calendari utente");
    }

    QDomDocument res;
    res.setContent(reply1->readAll());

    QString status = res.elementsByTagName("d:status").at(0).toElement().text();
    if (!status.contains("200")) {
      qWarning("Non riesco a ottenere il link ai calendari utente");
      ErrorManager::initializationError(
          this, "Il server non riesce a mandare il link ai calendari utente");
    }

    QString complete = client_->getHost().toString() +
                       res.elementsByTagName("d:href").at(1).toElement().text();
    QUrl calendars = QUrl(complete);
    client_->setUserCalendars(calendars);

    // get the link to every specific principal user's calendar
    auto reply2 = client_->listUserCalendars();
    connect(reply2, &QNetworkReply::finished, [reply2, this]() {
      if (reply2->error() != QNetworkReply::NoError) {
        qWarning("Non riesco a ottenere i calendari utente");
        ErrorManager::initializationError(
            this, "Il server non riesce a mandare i calendari utente");
      }

      QDomDocument res;
      res.setContent(reply2->readAll());

      auto statusesList = res.elementsByTagName("d:status");
      for (int i = 0; i < statusesList.length(); i++) {
        if (!statusesList.at(i).toElement().text().contains("200")) {
          qWarning("Non riesco a ottenere il link ai calendari utente");
          ErrorManager::initializationError(
              this,
              "Il server non riesce a mandare il link ai calendari utente");
        }
      }

      auto hrefList =
          res.elementsByTagName("d:href");  // first href must be ignored
      auto calendarNames = res.elementsByTagName("d:displayname");

      // ensure that it contains at least a calendar element in the CalDAV
      // namespace
      auto cal1 = res.elementsByTagName("cal:calendar");
      auto cal2 = res.elementsByTagName("c:calendar");
      if (cal1.length() != calendarNames.length() &&
          cal2.length() != calendarNames.length()) {
        QMessageBox::critical(this, "Errore", "Calendario non supportato");
        exit(UNSUPPORTED_ERROR);
      }

      // ensure that both VEVENT and VTODO are supported
      /*auto calComp1 = res.elementsByTagName("cal:comp");
      for(int i=0; i<calComp1.length(); i++){
          if(!calComp1.at(i).toElement().attribute("name").contains("VEVENT")){
              QMessageBox::critical(this, "Errore", "Un calendario non supporta
      gli eventi"); exit(-1);
          }
          if(!calComp1.at(i).toElement().attribute("name").contains("VTODO")){
              QMessageBox::critical(this, "Errore", "Un calendario non supporta
      le attività"); exit(-1);
          }
      }
      auto calComp2 = res.elementsByTagName("c:comp");
      for(int i=0; i<calComp2.length(); i++){
          if(!calComp2.at(i).toElement().attribute("name").contains("VEVENT")){
              QMessageBox::critical(this, "Errore", "Un calendario non supporta
      gli eventi"); exit(-1);
          }
          if(!calComp2.at(i).toElement().attribute("name").contains("VTODO")){
              QMessageBox::critical(this, "Errore", "Un calendario non supporta
      le attività"); exit(-1);
          }
      }*/

      // get calendar name and relative href
      for (int i = 0; i < calendarNames.length(); i++) {
        QString url = hrefList.at(i + 1).toElement().text();
        QString complete = client_->getHost().toString() + url;
        QUrl endpoint = QUrl(complete);
        client_->getUserCalendarsList().insert(
            calendarNames.at(i).toElement().text(), endpoint);
      }

      UserCalendarsChoice form(*client_);
      form.exec();
      initialize();
    });
  });
}

void MainWindow::initialize() {
  // Get allowed methods
  auto methods_reply = client_->findOutCalendarSupport();
  connect(methods_reply, &QNetworkReply::finished, [methods_reply, this]() {
    if (methods_reply->error() != QNetworkReply::NoError ||
        !methods_reply->hasRawHeader("Allow") ||
        !methods_reply->hasRawHeader("allow")) {
      qWarning("Non riesco a ottenere i metodi supportati dal server");
      ErrorManager::initializationError(
          this, "Il server non riesce a mandare i metodi supportati");
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
      if (props_reply->error() != QNetworkReply::NoError) {
        qWarning("Non riesco a ottenere le proprietà supportate dal server");
        ErrorManager::initializationError(
            this, "Il server non riesce a mandare le proprietà supportate");
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
          ErrorManager::initializationError(
              this, "Il server non riesce a mandare le proprietà supportate");
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

      refreshCalendarEvents();
      timer_->start(20000);
    });
  });
}

void MainWindow::refreshCalendarEvents() {
  QDate selected_date = ui->calendarWidget->selectedDate();
  QDate end_date = selected_date.addDays(ui->calendarTable->columnCount());

  ui->calendarTable->clearShowingWidgets();

  // prendo tutti gli eventi dalla data selezionata a una settimana dopo
  auto reply = client_->getDateRangeEvents(
      QDateTime(selected_date, QTime(0, 0)), QDateTime(end_date, QTime(0, 0)));
  connect(reply, &QNetworkReply::finished,
          [this, reply, selected_date, end_date]() {
            if (reply->error() != QNetworkReply::NoError) {
              qWarning("Non riesco a ottenere gli eventi dal server");
              QMessageBox::critical(
                  this, "Errore", "Il server non riesce a mandare gli eventi");
              exit(NETWORK_ERROR);
            }

            QDomDocument res;
            res.setContent(reply->readAll());

            auto statusesList = res.elementsByTagName("d:status");
            for (int i = 0; i < statusesList.length(); i++) {
              if (!statusesList.at(i).toElement().text().contains("200")) {
                qWarning("Non riesco a ottenere gli eventi dal server");
                QMessageBox::critical(
                    this, "Errore",
                    "Il server non riesce a mandare gli eventi");
                exit(NETWORK_ERROR);
              }
            }

            QDomNodeList responses = res.elementsByTagName("d:response");
            qDebug() << "Parsing events...";

            for (int i = 0; i < responses.length(); i++) {
              QDomElement current = responses.at(i).toElement();
              ICalendar tmp = ICalendar::fromXmlResponse(current);

              for (ICalendarComponent &ev : tmp.components()) {
                if (!ev.getEndDateTime() && !ev.getStartDateTime())
                  ui->taskList->createListWidget(std::move(ev));
                else if (!ev.getProperty("RRULE"))
                  ui->calendarTable->createTableItem(ev);
                else
                  getExpansion(std::move(ev));
              }
            }
            ready_event_ = true;
          });
}

void MainWindow::synchronize() {
  if (!ready_event_) {
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
      if (reply->error() != QNetworkReply::NoError) {
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
        refreshCalendarEvents();
      }
    });

  } else {
    auto reply2 = client_->receiveChangesBySyncToken();
    connect(reply2, &QNetworkReply::finished, [this, reply2]() {
      if (reply2->error() != QNetworkReply::NoError) {
        qWarning(
            "Non riesco a ottenere eventuali cambiamenti a eventi o attività "
            "dal server");
        QMessageBox::warning(this, "Errore di sincronizzazione",
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
          QMessageBox::warning(this, "Errore di inizializzazione",
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
          ICalendar cal = ICalendar::fromXmlResponse(current);
          for (ICalendarComponent &event : cal.components()) {
            if (!event.getStartDateTime() && !event.getEndDateTime())
              ui->taskList->createListWidget(std::move(event));
            else if (!event.getProperty("RRULE"))
              ui->calendarTable->createTableItem(event);
            else
              getExpansion(std::move(event));
          }
        }
        if (status.contains("404")) {
          QString href =
              xml.elementsByTagName("d:href").at(0).toElement().text();
          ui->taskList->removeByHref(href);
          ui->calendarTable->removeByHref(href);
        };
      }
    });
  }
}

void MainWindow::changeUserServer() {
  Userform form(*client_);
  form.exec();
  if (client_->getPrincipal().isEmpty()) {
    tryGetPrincipal();
  } else {
    getUserCalendars();
  }
}

void MainWindow::changeCalendar() {
  if (client_->getUserCalendarsList().isEmpty()) {
    QMessageBox::warning(
        this, "Attendi ancora un pò",
        "Il server non è ancora pronto, riprova fra qualche istante");
  } else {
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
              ICalendar tmp = ICalendar::fromXmlResponse(current);

              for (ICalendarComponent &ev : tmp.components()) {
                ICalendarComponent copy(moved);
                if (ev.getStartDateTime())
                  copy.setStartDateTime(ev.getStartDateTime().value());
                if (ev.getEndDateTime())
                  copy.setEndDateTime(ev.getEndDateTime().value());
                ui->calendarTable->createTableItem(copy);
              }
            }
          });
}

void MainWindow::showEditForm(ICalendarComponent component) {
  bool existing =
      ui->calendarTable->getShowingEvents().contains(component.getUID()) ||
      ui->taskList->getShowingComponents().contains(component.getUID());
  CreateEventForm form(&component, *client_, existing, this);
  connect(&form, &CreateEventForm::accepted, this, &MainWindow::synchronize);
  form.exec();
}

MainWindow::~MainWindow() { delete ui; }
