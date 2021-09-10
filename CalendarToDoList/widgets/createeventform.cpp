#include "createeventform.h"

#include "CalendarClient/CalendarClient.h"
#include "calendartable.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client,
                                 ICalendar& calendar, bool existing,
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
  if (isEvent)
    ui->typeSelection->setCurrentText("Evento");
  else
    ui->typeSelection->setCurrentText("Attività");

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

      // nuovo evento
      if (isEvent_) {
        QDateTime max = ui->endDateTime->dateTime();
        if (ui->startDateTime->dateTime() > max) {
          max.setTime(ui->startDateTime->time());
          event_->setStartDateTime(max);
          qDebug() << "Date: " + event_->startDateTime().toString() + "     " +
                          event_->startDateTime().toString();
        }
        auto reply = client_->saveElement(*event_);
        connect(reply, &QNetworkReply::finished, [this]() {
          // imposto il nuovo eTag dell'evento
          auto reply1 = client_->getElementByUID(event_->uid(), true);
          connect(reply1, &QNetworkReply::finished, [reply1, this]() {
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto href_list = res.elementsByTagName("d:href");
            auto eTagList = res.elementsByTagName("d:getetag");
            event_->setETag(eTagList.at(0).toElement().text());
            event_->setHref(href_list.at(0).toElement().text());
            client_->eTags().insert(href_list.at(0).toElement().text(),
                                    eTagList.at(0).toElement().text());
            qDebug() << "New event saved\n";
            emit requestView();
            accept();
          });
        });
      }

      //nuovo task
      else{
          Task* task = dynamic_cast<Task*>(event_);
        auto reply = client_->saveElement(*task);
        connect(reply, &QNetworkReply::finished, [this, task]() {
          // imposto il nuovo eTag del task
          auto reply1 = client_->getElementByUID(task->uid(), false);
          connect(reply1, &QNetworkReply::finished, [reply1, task, this]() {
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto href_list = res.elementsByTagName("d:href");
            auto eTagList = res.elementsByTagName("d:getetag");
            task->setETag(eTagList.at(0).toElement().text());
            task->setHref(href_list.at(0).toElement().text());
            client_->eTags().insert(href_list.at(0).toElement().text(),
                                    eTagList.at(0).toElement().text());
            qDebug() << "New task saved\n";
            emit requestView();
            accept();
          });
        });
      }

    }

    else {

      //update di un evento
      if (isEvent_) {

          for (CalendarEvent& ev : calendar_->events()) {
            qDebug() << "Ev prima: " + ev.summary();
          }

        QString hrefToUpdate = event_->href();
        // elimino l'evento vecchio dalla lista
        for (CalendarEvent& ev : calendar_->events()) {
          if (ev.href() == hrefToUpdate) {
            calendar_->events().removeOne(ev);
          }
        }

        for (CalendarEvent& ev : calendar_->events()) {
          qDebug() << "Ev dopo: " + ev.summary();
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
            auto reply1 = client_->getElementByUID(event_->uid(), true);
            connect(reply1, &QNetworkReply::finished,
                    [reply1, this, hrefToUpdate]() {
                      QDomDocument res;
                      res.setContent(reply1->readAll());
                      auto href_list = res.elementsByTagName("d:href");
                      auto eTagList = res.elementsByTagName("d:getetag");
                      (*event_).setETag(eTagList.at(0).toElement().text());
                      client_->eTags().insert(
                          hrefToUpdate, eTagList.at(0).toElement().text());
                      calendar_->events().append(*event_);
                    });
          }
          qDebug() << "Event updated\n";
        });
      }

      //update di un task
      else {
        Task* task = dynamic_cast<Task*>(event_);
        // elimino il task vecchio dalla lista
        QString hrefToUpdate = task->href();
        /*
        for (Task& t : calendar_->tasks()) {
          if (t.href() == hrefToUpdate) {
            calendar_->tasks().removeOne(t);
          }
        }
        */
        // elimino il vecchio eTag dalla lista
        for (auto el : client_->eTags().keys()) {
          if (el == hrefToUpdate) {
            client_->eTags().remove(el);
          }
        }
        auto reply = client_->updateElement(*task, task->eTag());
        connect(reply, &QNetworkReply::finished,
                [this, reply, task, hrefToUpdate]() {
                  // imposto il nuovo eTag dell'evento
                  bool flag = false;
                  for (auto el : reply->rawHeaderPairs()) {
                    if (el.first ==
                        "ETag") {  // eTag direttamente restituito dal server
                      (*event_).setETag(el.second);
                      client_->eTags().insert(hrefToUpdate, el.second);
                      // calendar_->tasks().append(*task);
                      flag = true;
                    }
                  }
                  if (!flag) {
                    auto reply1 =
                        nullptr;  // client_->getElementByUID(task->uid());
                    connect(
                        reply1, &QNetworkReply::finished,
                        [reply1, this, task, hrefToUpdate]() {
                          QDomDocument res;
                          // res.setContent(reply1->readAll());
                          auto href_list = res.elementsByTagName("d:href");
                          auto eTagList = res.elementsByTagName("d:getetag");
                          (*task).setETag(eTagList.at(0).toElement().text());
                          client_->eTags().insert(
                              hrefToUpdate, eTagList.at(0).toElement().text());
                          // calendar_->tasks().append(*task);
                        });
                  }
                });
      }
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
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        QString hrefToDelete = task->href();
        QString eTag = task->eTag();
        // calendar_->tasks().removeOne(*task);
        client_->deleteETag(hrefToDelete);
        client_->deleteElement(*task, eTag);
      }
      emit requestView();
    }
    accept();
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
      } else {
        Task* task = dynamic_cast<Task*>(event_);
        QString hrefToDelete = task->href();
        QString eTag = task->eTag();
        // calendar_->tasks().removeOne(*task);
        client_->deleteETag(hrefToDelete);
        client_->deleteElement(*task, eTag);
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
    ui->endDateTime->hide();
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
    ui->endDateTime->show();
  }
}
