#include "createeventform.h"

#include "CalendarClient/CalendarClient.h"
#include "calendartable.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client,
                                 ICalendar& calendar, bool existing,
                                 QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateEventForm),
      event_(event),
      client_(&client),
      calendar_(&calendar),
      existing_(existing) {
  ui->setupUi(this);

  if (existing_) ui->typeSelection->hide();
  resetFormFields();

  connect(ui->typeSelection, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (text == "Evento")
              event_ = new CalendarEvent();
            else
              event_ = new Task();

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
            if (text == "Non si ripete") event_->setRRULE("");

            if (text == "Ogni giorno") event_->setRRULE("FREQ=DAILY");

            if (text == "Ogni settimana")
              event_->setRRULE(QString("FREQ=WEEKLY; BYDAY=%1")
                                   .arg(CalendarEvent::stringFromWeekDay(
                                       start_date.dayOfWeek())));

            if (text == "Ogni mese")
              event_->setRRULE(
                  QString("FREQ=MONTHLY; BYMONTHDAY=%1").arg(start_date.day()));

            if (text == "Ogni anno") event_->setRRULE("FREQ=YEARLY");
          });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    /* COMPARING CREATE SOME ERROR
    QDateTime max = ui->endDateTime->dateTime();
    if (ui->startDateTime->dateTime() > max) {
      max.setTime(ui->startDateTime->time());
      event_->setStartDateTime(max);
      qDebug() << "Date: " + event_->startDateTime().toString() + "     " +
                      event_->startDateTime().toString();
    }
    */
    // NEW EVENT
    if (!existing_) {
      auto reply = client_->saveElement(*event_);
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        // imposto il nuovo eTag dell'evento
        auto reply1 = client_->getElementByUID(event_->uid());
        connect(reply1, &QNetworkReply::finished, [reply1, this]() {
          QDomDocument res;
          res.setContent(reply1->readAll());
          auto eTagList = res.elementsByTagName("d:getetag");
          event_->setETag(eTagList.at(0).toElement().text());
          qDebug() << "New event saved\n";
          emit requestView();
          accept();
        });
      });
    }
    // UPDATE EVENT
    else {
      auto reply = client_->updateElement(*event_, event_->eTag());
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->hasRawHeader("ETag")) {
          event_->setETag(reply->rawHeader("ETag"));
          accept();
        } else {
          auto reply1 = client_->getElementByUID(event_->uid());
          connect(reply1, &QNetworkReply::finished, [reply1, this]() {
            QDomDocument res;
            res.setContent(reply1->readAll());
            auto eTagList = res.elementsByTagName("d:getetag");
            event_->setETag(eTagList.at(0).toElement().text());
            accept();
          });
        }
        qDebug() << "Event updated\n";
      });
    }
  });
  connect(ui->deleteButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      qDebug() << "Event not deleted\n";
      reject();
    } else {
      auto reply = client_->deleteElement(*event_, event_->eTag());
      connect(reply, &QNetworkReply::finished, [this, reply]() { done(2); });
    }
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
    ui->typeSelection->setCurrentText("Attività");
    qDebug() << "Task";
  } else {
    ui->completionButton->hide();
    // Show
    ui->RRule->show();
    ui->locationEdit->show();
    ui->endDateTime->show();
    ui->typeSelection->setCurrentText("Evento");
  }
}
