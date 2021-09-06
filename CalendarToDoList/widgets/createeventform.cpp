#include "createeventform.h"

#include "CalendarClient/CalendarClient.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client,
                                 Calendar& calendar, bool existing,
                                 bool isEvent, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateEventForm),
      event_(event),
      client_(&client),
      calendar_(&calendar),
      existing_(existing),
      isEvent_(isEvent) {
  ui->setupUi(this);

  resetFormFields();

  if (existing_) ui->typeSelection->hide();

  // TYPE
  if (isEvent) {
    ui->typeSelection->setCurrentText("Evento");
  } else {
    ui->typeSelection->setCurrentText("Attività");
  }
  connect(ui->typeSelection, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (text == "Evento") {
              if (!existing_) {
                event_ = new CalendarEvent;
              }
              isEvent_ = true;
            } else {
              if (!existing_) {
                event_ = new Task;
              }
              isEvent_ = false;
            }
            resetFormFields();
          });

  // SUMMARY
  connect(ui->titleEdit, &QLineEdit::textChanged,
          [this](const QString& text) { event_->setSummary(text); });
  // LOCATION
  connect(ui->locationEdit, &QLineEdit::textChanged,
          [this](const QString& text) { event_->setLocation(text); });
  // DESCRIPTION
  connect(ui->descriptionEdit, &QTextEdit::textChanged,
          [this]() { event_->setDescription(ui->descriptionEdit->toHtml()); });
  // ALLDAY
  connect(ui->allDayBox, &QCheckBox::stateChanged, [this](int state) {
    event_->setAllDay(state == Qt::CheckState::Checked);
  });
  // COMPLETION
  connect(ui->completionButton, &QPushButton::clicked, [this]() {
    Task* task = dynamic_cast<Task*>(event_);
    task->flipCompleted();
    QString text = task->completed().first ? "Segna come non completata"
                                           : "Segna come completata";
    ui->completionButton->setText(text);
  });
  // SDATETIME
  connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged,
          [this](const QDateTime& datetime) {
            event_->setStartDateTime(datetime);
          });
  // EDATETIME
  connect(
      ui->endDateTime, &QDateTimeEdit::dateTimeChanged,
      [this](const QDateTime& datetime) { event_->setEndDateTime(datetime); });
  // RRULE
  connect(ui->RRule, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            QDate start_date = event_->startDateTime().date();
            if (text == "Non si ripete") {
              event_->setRRULE("");
            }
            if (text == "Ogni giorno") {
              event_->setRRULE("FREQ=DAILY");
            }
            if (text == "Ogni settimana") {
              event_->setRRULE(QString("FREQ=WEEKLY; BYDAY=%1")
                                   .arg(CalendarEvent::stringFromWeekDay(
                                       start_date.dayOfWeek())));
            }
            if (text == "Ogni mese") {
              event_->setRRULE(
                  QString("FREQ=MONTHLY; BYMONTHDAY=%1").arg(start_date.day()));
            }
            if (text == "Ogni anno") {
              event_->setRRULE("FREQ=YEARLY");
            }
          });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      if (isEvent_) {
        QDateTime max = ui->endDateTime->dateTime();
        if (ui->startDateTime->dateTime() > max) {
          max.setTime(ui->startDateTime->time());
          (*event_).setStartDateTime(max);
          qDebug() << "Date: " + event_->startDateTime().toString() + "     " +
                          event_->startDateTime().toString();
        }
        auto reply = client_->saveElement(*event_);
        connect(reply, &QNetworkReply::finished, [this]() {
          // imposto il nuovo eTag dell'evento
          auto reply1 = client_->lookForChanges();
          connect(reply1, &QNetworkReply::finished, [reply1, this]() {
            QSet<QString> allETags;
            for (auto ev : calendar_->events()) {
              allETags.insert(ev.eTag());
            }
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto href_list = res.elementsByTagName("d:href");
            auto eTagList = res.elementsByTagName("d:getetag");
            for (int i = 0; i < eTagList.size(); i++) {
              if (!allETags.contains(eTagList.at(i).toElement().text())) {
                (*event_).setETag(eTagList.at(i).toElement().text());
                (*event_).setHref(href_list.at(i).toElement().text());
                client_->eTags().insert(href_list.at(i).toElement().text(),
                                        eTagList.at(i).toElement().text());
                break;
              }
            }
            calendar_->events().append(*event_);
            qDebug() << "New event saved\n";
          });
        });
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        auto reply = client_->saveElement(*task);
        connect(reply, &QNetworkReply::finished, [this, task]() {
          // imposto il nuovo eTag del task
          auto reply1 = client_->lookForChanges();
          connect(reply1, &QNetworkReply::finished, [reply1, this, task]() {
            QSet<QString> allETags;
            for (auto ev : calendar_->events()) {
              allETags.insert(ev.eTag());
            }
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto href_list = res.elementsByTagName("d:href");
            auto eTagList = res.elementsByTagName("d:getetag");
            for (int i = 0; i < eTagList.size(); i++) {
              if (!allETags.contains(eTagList.at(i).toElement().text())) {
                task->setETag(eTagList.at(i).toElement().text());
                task->setHref(href_list.at(i).toElement().text());
                client_->eTags().insert(href_list.at(i).toElement().text(),
                                        eTagList.at(i).toElement().text());
                break;
              }
            }
            calendar_->tasks().append(*task);
            qDebug() << "New task saved\n";
          });
        });
      }
    } else {
      if (isEvent_) {
        QString hrefToUpdate = event_->href();
        // elimino l'evento vecchio dalla lista
        for (CalendarEvent& ev : calendar_->events()) {
          if (ev.href() == hrefToUpdate) {
            calendar_->events().removeOne(ev);
          }
        }
        // elimino il vecchio eTag dalla lista
        for (auto el : client_->eTags().keys()) {
          if (el == hrefToUpdate) {
            client_->eTags().remove(el);
          }
        }
        auto reply = client_->updateElement(*event_, event_->eTag());
        connect(reply, &QNetworkReply::finished, [this, reply, hrefToUpdate]() {
          // imposto il nuovo eTag dell'evento
          bool flag = false;
          for (auto el : reply->rawHeaderPairs()) {
            if (el.first ==
                "ETag") {  // eTag direttamente restituito dal server
              (*event_).setETag(el.second);
              calendar_->events().append(*event_);
              client_->eTags().insert(hrefToUpdate, el.second);
              flag = true;
            }
          }
          if (!flag) {
            auto reply1 = client_->lookForChanges();
            connect(
                reply1, &QNetworkReply::finished,
                [reply1, this, hrefToUpdate]() {
                  QSet<QString> allETags;
                  for (auto ev : calendar_->events()) {
                    allETags.insert(ev.eTag());
                  }
                  QDomDocument res;
                  res.setContent(reply1->readAll());
                  auto href_list = res.elementsByTagName("d:href");
                  auto eTagList = res.elementsByTagName("d:getetag");
                  for (int i = 0; i < eTagList.size(); i++) {
                    if (!allETags.contains(eTagList.at(i).toElement().text())) {
                      (*event_).setETag(eTagList.at(i).toElement().text());
                      client_->eTags().insert(
                          hrefToUpdate, eTagList.at(i).toElement().text());
                      break;
                    }
                  }
                  calendar_->events().append(*event_);
                  qDebug() << "Event updated\n";
                });
          }
        });
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        // elimino il task vecchio dalla lista
        QString hrefToUpdate = task->href();
        for (Task& t : calendar_->tasks()) {
          if (t.href() == hrefToUpdate) {
            calendar_->tasks().removeOne(t);
          }
        }
        // elimino il vecchio eTag dalla lista
        for (auto el : client_->eTags().keys()) {
          if (el == hrefToUpdate) {
            client_->eTags().remove(el);
          }
        }
        auto reply = client_->updateElement(*task, task->eTag());
        connect(
            reply, &QNetworkReply::finished,
            [this, reply, task, hrefToUpdate]() {
              // imposto il nuovo eTag dell'evento
              bool flag = false;
              for (auto el : reply->rawHeaderPairs()) {
                if (el.first ==
                    "ETag") {  // eTag direttamente restituito dal server
                  (*event_).setETag(el.second);
                  client_->eTags().insert(hrefToUpdate, el.second);
                  calendar_->tasks().append(*task);
                  flag = true;
                }
              }
              if (!flag) {
                auto reply1 = client_->lookForChanges();
                connect(
                    reply1, &QNetworkReply::finished,
                    [reply1, this, task, hrefToUpdate]() {
                      QSet<QString> allETags;
                      for (auto t : calendar_->tasks()) {
                        allETags.insert(t.eTag());
                      }
                      QDomDocument res;
                      res.setContent(reply1->readAll());
                      auto href_list = res.elementsByTagName("d:href");
                      auto eTagList = res.elementsByTagName("d:getetag");
                      for (int i = 0; i < eTagList.size(); i++) {
                        if (!allETags.contains(
                                eTagList.at(i).toElement().text())) {
                          (*task).setETag(eTagList.at(i).toElement().text());
                          client_->eTags().insert(
                              hrefToUpdate, eTagList.at(i).toElement().text());
                          break;
                        }
                      }
                      calendar_->tasks().append(*task);
                      qDebug() << "Task updated\n";
                    });
              }
            });
      }
    }
    emit requestView();
    accept();
  });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      if (isEvent_) {
        QDateTime max = ui->endDateTime->dateTime();
        if (ui->startDateTime->dateTime() > max) {
          max.setTime(ui->startDateTime->time());
          (*event_).setStartDateTime(max);
          qDebug() << "Date: " + event_->startDateTime().toString() + "     " +
                          event_->startDateTime().toString();
        }
        auto reply = client_->saveElement(*event_);
        connect(reply, &QNetworkReply::finished, [this]() {
          // imposto il nuovo eTag dell'evento
          auto reply1 = client_->lookForChanges();
          connect(reply1, &QNetworkReply::finished, [reply1, this]() {
            QSet<QString> allETags;
            for (auto ev : calendar_->events()) {
              allETags.insert(ev.eTag());
            }
            event_ = nullptr;
            if (isEvent_) {
              QString hrefToDelete = event_->href();
              QString eTag = event_->eTag();
              calendar_->events().removeOne(*event_);
              client_->deleteETag(hrefToDelete);
              client_->deleteElement(*event_, eTag);
              qDebug() << "Event " + event_->summary() + " deleted\n";
            } else {
              Task* task = dynamic_cast<Task*>(event_);
              QString hrefToDelete = task->href();
              QString eTag = task->eTag();
              calendar_->tasks().removeOne(*task);
              client_->deleteETag(hrefToDelete);
              client_->deleteElement(*task, eTag);
              qDebug() << "Task " + (*task).summary() + " deleted\n";
            }
            calendar_->events().append(*event_);
            qDebug() << "New event saved\n";
          });
        });
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        auto reply = client_->saveElement(*task);
        connect(reply, &QNetworkReply::finished, [this, task]() {
          // imposto il nuovo eTag del task
          auto reply1 = client_->lookForChanges();
          connect(reply1, &QNetworkReply::finished, [reply1, this, task]() {
            QSet<QString> allETags;
            for (auto ev : calendar_->events()) {
              allETags.insert(ev.eTag());
            }
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto href_list = res.elementsByTagName("d:href");
            auto eTagList = res.elementsByTagName("d:getetag");
            for (int i = 0; i < eTagList.size(); i++) {
              if (!allETags.contains(eTagList.at(i).toElement().text())) {
                task->setETag(eTagList.at(i).toElement().text());
                task->setHref(href_list.at(i).toElement().text());
                break;
              }
            }
            calendar_->tasks().append(*task);
            for (auto ev : calendar_->tasks()) {
              qDebug() << "Tasks: " + ev.summary() + "    " + ev.href() +
                              "   " + ev.eTag();
            }
            qDebug() << "New task saved\n";
          });
        });
      }
    } else {
      if (isEvent_) {
        QString hrefToUpdate = event_->href();
        qDebug() << "\n\nETag: " + client_->eTags().find(hrefToUpdate).value() +
                        "\n\n";
        client_->updateElement(*event_,
                               client_->eTags().find(hrefToUpdate).value());
        for (CalendarEvent& ev : calendar_->events()) {
          if (ev.href() == hrefToUpdate) {
            calendar_->events().removeOne(ev);
          }
        }
      } else {
        // list.getTasks().removeOne(t);
        Task* task = dynamic_cast<Task*>(event_);
        // auto reply = client_->updateTask(list, *task);
        // list.getTasks().append(*task);
        qDebug() << "Task " + task->summary() + " saved\n";
      }
      emit requestView();
      accept();
    }
  });

  connect(ui->deleteButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      if (isEvent_) {
        qDebug() << "Event not created\n";
      } else {
        qDebug() << "Task not created\n";
      }
      event_ = nullptr;
    } else {
      if (isEvent_) {
        QString hrefToDelete = event_->href();
        QString eTag = event_->eTag();
        calendar_->events().removeOne(*event_);
        client_->deleteETag(hrefToDelete);
        client_->deleteElement(*event_, eTag);
        qDebug() << "Event " + event_->summary() + " deleted\n";
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        // client_->deleteTask(list, *task);
        // list.getTasks().removeOne(*task);
        qDebug() << "Task " + (*task).summary() + " deleted\n";
      }
      emit requestView();
    }
    accept();
  });
}

CreateEventForm::~CreateEventForm() { delete ui; }

void CreateEventForm::resetFormFields() {
  ui->titleEdit->setText(event_->summary());
  ui->startDateTime->setDateTime(event_->startDateTime());
  ui->endDateTime->setDateTime(event_->endDateTime());
  ui->allDayBox->setChecked(event_->all_day());
  ui->locationEdit->setText(event_->location());

  Task* task = dynamic_cast<Task*>(event_);
  if (task != nullptr) {
    ui->RRule->hide();
    ui->locationEdit->hide();
    ui->startDateTime->hide();
    // Show
    ui->completionButton->show();
    // Completion button
    QString text = task->completed().first ? "Segna come non completata"
                                           : "Segna come completata";
    ui->completionButton->setText(text);
  } else {
    ui->completionButton->hide();
    // Show
    ui->RRule->show();
    ui->locationEdit->show();
    ui->startDateTime->show();
  }
}
