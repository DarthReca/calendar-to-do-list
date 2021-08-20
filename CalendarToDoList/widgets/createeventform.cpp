#include "createeventform.h"

#include "CalendarClient/calendarclient.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client,
                                 Calendar& calendar, bool existing,
                                 QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateEventForm),
      event_(event),
      client_(&client),
      calendar_(&calendar),
      existing_(existing) {
  ui->setupUi(this);

  ui->titleEdit->setText(event_->summary());
  ui->locationEdit->setText(event_->location());
  ui->startDateTime->setDateTime(event_->getStartDateTime());
  ui->endDateTime->setDateTime(event_->getEndDateTime());

  connect(ui->titleEdit, &QLineEdit::textChanged,
          [this](const QString& text) { event_->setSummary(text); });
  connect(ui->locationEdit, &QLineEdit::textChanged,
          [this](const QString& text) { event_->setLocation(text); });
  connect(ui->descriptionEdit, &QTextEdit::textChanged,
          [this]() { event_->setDescription(ui->descriptionEdit->toHtml()); });

  connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged,
          [this](const QDateTime& datetime) {
            event_->setStartDateTime(datetime);
          });
  connect(
      ui->endDateTime, &QDateTimeEdit::dateTimeChanged,
      [this](const QDateTime& datetime) { event_->setEndDateTime(datetime); });

  connect(ui->RRule, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (text == "Non si ripete") {
              event_->setRRULE("");
            }
            if (text == "Ogni giorno") {
              event_->setRRULE("FREQ=DAILY");
            }
            if (text == "Ogni settimana") {
              event_->setRRULE("FREQ=WEEKLY");
            }
            if (text == "Ogni mese") {
              event_->setRRULE("FREQ=MONTHLY");
            }
            if (text == "Ogni anno") {
              event_->setRRULE("FREQ=YEARLY");
            }
          });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      client_->saveEvent(*event_);
      calendar_->events().append(*event_);
      qDebug() << "New event saved\n";
    } else {
      QString hrefToUpdate = event_->getHREF();
      client_->updateEvent(*event_,
                           client_->getETags().find(hrefToUpdate).value());
      for (CalendarEvent& ev : calendar_->events()) {
        if (ev.getHREF() == hrefToUpdate) {
          calendar_->events().removeOne(ev);
        }
      }
      calendar_->events().append(*event_);
      qDebug() << "Event " + event_->summary() + " updated\n";
    }
    emit requestView();
    accept();
  });

  connect(ui->deleteButton, &QPushButton::clicked, [this] {
    if (!existing_) {
      event_.clear();
      qDebug() << "Event not created\n";
    } else {
      QString hrefToDelete = event_->getHREF();
      QString eTag = client_->getETags().find(hrefToDelete).value();
      calendar_->events().removeOne(*event_);
      client_->deleteETag(hrefToDelete);
      client_->deleteEvent(*event_, eTag);
      qDebug() << "Event " + event_->summary() + " deleted\n";
      emit requestView();
    }
    accept();
  });
}

CreateEventForm::~CreateEventForm() { delete ui; }

void CreateEventForm::on_allDayBox_stateChanged(int arg1) {
  if (arg1 == Qt::CheckState::Checked) {
  } else {
  }
}
