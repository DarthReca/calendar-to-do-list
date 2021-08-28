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

  ResetFormFields();

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
            ResetFormFields();
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
    event_->setAll_day(state == Qt::CheckState::Checked);
  });
  // COMPLETION
  connect(ui->completionButton, &QPushButton::clicked, [this]() {
    Task* task = qobject_cast<Task*>(event_);
    task->FlipCompleted();
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
            QDate start_date = event_->getStartDateTime().date();
            if (text == "Non si ripete") {
              event_->setRRULE("");
            }
            if (text == "Ogni giorno") {
              event_->setRRULE("FREQ=DAILY");
            }
            if (text == "Ogni settimana") {
              event_->setRRULE(QString("FREQ=WEEKLY; BYDAY=%1")
                                   .arg(CalendarEvent::StringFromWeekDay(
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
          qDebug() << event_->getStartDateTime().toString() + "     " +
                          event_->getStartDateTime().toString();
        }
        client_->saveElement(*event_);

        auto reply = client_->lookForChanges();
        connect(reply, &QNetworkReply::finished, [reply, this]() {
          QString hrefToSearch = (*event_).getHREF();
          QDomDocument res;
          res.setContent(reply->readAll());
          auto href_list = res.elementsByTagName("D:href");
          auto eTagList = res.elementsByTagName("D:getetag");
          for (int i = 0; i < href_list.size(); i++) {
            if (href_list.at(i).toElement().text() == hrefToSearch) {
              (*event_).setETag(eTagList.at(i).toElement().text());
              break;
            }
          }
          qDebug() << (*event_).eTag();
          calendar_->events().append(*event_);
        });
        qDebug() << "New event saved\n";
      } else {
          Task* task = qobject_cast<Task*>(event_);
          //auto reply = client_->createTask(list, *task);
          //list.getTasks().append(*task);
        }
        qDebug() << "New task saved\n";
    } else {
      if (isEvent_) {
        QString hrefToUpdate = event_->getHREF();
        qDebug() << "\n\nETag: " +
                        client_->getETags().find(hrefToUpdate).value() + "\n\n";
        client_->updateElement(*event_,
                             client_->getETags().find(hrefToUpdate).value());
        for (CalendarEvent& ev : calendar_->events()) {
          if (ev.getHREF() == hrefToUpdate) {
            calendar_->events().removeOne(ev);
          }
        }
      } else {
           //list.getTasks().removeOne(t);
           Task* task = qobject_cast<Task*>(event_);
           //auto reply = client_->updateTask(list, *task);
            //list.getTasks().append(*task);
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
      event_.clear();
    } else {
      if (isEvent_) {
        QString hrefToDelete = event_->getHREF();
        QString eTag = event_->eTag();
        calendar_->events().removeOne(*event_);
        client_->deleteETag(hrefToDelete);
        client_->deleteElement(*event_, eTag);
        qDebug() << "Event " + event_->summary() + " deleted\n";
      } else {
          Task* task = qobject_cast<Task*>(event_);
          //client_->deleteTask(list, *task);
          //list.getTasks().removeOne(*task);
          qDebug() << "Task " + (*task).summary() + " deleted\n";
      }
      emit requestView();
    }
    accept();
  });
}

CreateEventForm::~CreateEventForm() { delete ui; }

void CreateEventForm::ResetFormFields() {
  ui->titleEdit->setText(event_->summary());
  ui->startDateTime->setDateTime(event_->getStartDateTime());
  ui->endDateTime->setDateTime(event_->getEndDateTime());
  ui->allDayBox->setChecked(event_->all_day());
  ui->locationEdit->setText(event_->location());

  QPointer<Task> task = qobject_cast<Task*>(event_);
  if (!task.isNull()) {
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
