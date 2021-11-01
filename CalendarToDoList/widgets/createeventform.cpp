#include "createeventform.h"

#include <QMessageBox>

#include "calendarclient.h"
#include "calendartable.h"
#include "ui_createeventform.h"
#include "errormanager.h"

CreateEventForm::CreateEventForm(ICalendarComponent* event,
                                 CalendarClient& client, bool existing,
                                 QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateEventForm),
      component_(event),
      client_(&client),
      existing_(existing) {
  ui->setupUi(this);

  if (existing_) ui->typeSelection->hide();
  if (existing_ && component_->type() == "VTODO" &&
      !component_->getEndDateTime()) {
    for (int i = 0; i < ui->gridLayout->count(); i++)
      ui->gridLayout->itemAt(i)->widget()->hide();
  }
  resetFormFields();

  connect(ui->typeSelection, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (text == "Evento")
              component_->setType("VEVENT");
            else
              component_->setType("VTODO");

            resetFormFields();
          });

  // SUMMARY
  connect(ui->titleEdit, &QLineEdit::textChanged, [this](const QString& text) {
    component_->setProperty("SUMMARY", text);
  });
  // LOCATION
  connect(ui->locationEdit, &QLineEdit::textChanged,
          [this](const QString& text) {
            component_->setProperty("LOCATION", text);
          });
  // DESCRIPTION
  connect(ui->descriptionEdit, &QTextEdit::textChanged, [this]() {
    component_->setProperty("DESCRIPTION", ui->descriptionEdit->toPlainText());
  });
  // ALLDAY
  connect(ui->allDayBox, &QCheckBox::stateChanged, [this](int state) {
    component_->setAllDay(state == Qt::CheckState::Checked);
  });
  // COMPLETION
  connect(ui->completionButton, &QPushButton::clicked, [this]() {
    if (component_->getProperty("COMPLETED")) {
      component_->removeProperty("COMPLETED");
      ui->completionButton->setText("Segna come completata");
    } else {
      component_->setProperty(
          "COMPLETED",
          QDateTime::currentDateTimeUtc().toString("yyyyMMdd'T'hhmmss'Z'"));
      ui->completionButton->setText("Segna come non completata");
    }
  });
  // SDATETIME
  connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged,
          [this](const QDateTime& datetime) {
            component_->setStartDateTime(datetime);
          });
  // EDATETIME
  connect(ui->endDateTime, &QDateTimeEdit::dateTimeChanged,
          [this](const QDateTime& datetime) {
            component_->setEndDateTime(datetime);
          });
  // RRULE
  connect(ui->RRule, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (!component_->getStartDateTime())
              component_->setStartDateTime(ui->startDateTime->dateTime());
            QDate start_date = component_->getStartDateTime().value().date();
            if (text == "Non si ripete") component_->removeProperty("RRULE");

            if (text == "Ogni giorno")
              component_->setProperty("RRULE", "FREQ=DAILY");

            if (text == "Ogni settimana")
              component_->setProperty(
                  "RRULE", QString("FREQ=WEEKLY; BYDAY=%1")
                               .arg(ICalendarComponent::stringFromWeekDay(
                                   start_date.dayOfWeek())));

            if (text == "Ogni mese")
              component_->setProperty(
                  "RRULE",
                  QString("FREQ=MONTHLY; BYMONTHDAY=%1").arg(start_date.day()));

            if (text == "Ogni anno")
              component_->setProperty("RRULE", "FREQ=YEARLY");
          });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    if (component_->type() == "VEVENT") {
      if (ui->startDateTime->date() > ui->endDateTime->date()) {
        QMessageBox::critical(this, "Error in date selection",
                              "The start date must be before the end date!");
        return;
      } else {
        if (ui->startDateTime->time() > ui->endDateTime->time()) {
          QMessageBox::critical(this, "Error in time selection",
                                "The start time must be before the end time!");
          return;
        }
      }
    }

    // NEW EVENT
    if (!existing_) {
      auto reply = client_->saveElement(*component_);
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
          ErrorManager::creationError(this, "Il server non accetta il nuovo elemento");
        }
        qDebug() << reply->readAll();
        accept();
      });
    }
    // UPDATE EVENT
    else {
      auto reply = client_->updateElement(*component_, component_->eTag());
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
          ErrorManager::creationError(this, "Il server non accetta l'aggiornamento dell'elemento selezionato");
        }
        accept();
      });
    }
  });
  connect(ui->deleteButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      reject();
    } else {
      auto reply = client_->deleteElement(*component_, component_->eTag());
      connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
          ErrorManager::creationError(this, "Il server non accetta l'eliminazione dell'elemento selezionato");
        }
      });
      accept();
    }
  });
}

CreateEventForm::~CreateEventForm() { delete ui; }

void CreateEventForm::resetFormFields() {
  ui->titleEdit->setText(component_->getProperty("SUMMARY").value_or(""));
  ui->startDateTime->setDateTime(
      component_->getStartDateTime().value_or(QDateTime::currentDateTime()));
  ui->endDateTime->setDateTime(component_->getEndDateTime().value_or(
      QDateTime::currentDateTime().addSecs(60 * 60)));
  ui->allDayBox->setChecked(component_->allDay());
  ui->locationEdit->setText(component_->getProperty("LOCATION").value_or(""));
  ui->descriptionEdit->setText(component_->getProperty("DESCRIPTION").value_or(""));

  component_->setEndDateTime(ui->endDateTime->dateTime());

  if (component_->type() == "VTODO") {
    ui->RRule->hide();
    ui->locationEdit->hide();
    ui->startDateTime->hide();
    // Show
    ui->completionButton->show();
    // Completion button
    QString text = component_->getProperty("COMPLETED")
                       ? "Segna come non completata"
                       : "Segna come completata";
    ui->completionButton->setText(text);
    ui->typeSelection->setCurrentText("Attività");

    component_->removeProperty("DTSTART");
  } else {
    ui->completionButton->hide();
    // Show
    ui->RRule->show();
    ui->locationEdit->show();
    ui->startDateTime->show();
    ui->typeSelection->setCurrentText("Evento");

    component_->setStartDateTime(ui->startDateTime->dateTime());
  }

  // TEMPORARY
  ui->RRule->hide();
}
