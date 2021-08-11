#include "createeventform.h"
#include "ui_createeventform.h"
#include "CalendarClient/CalendarClient_CalDAV.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, QOAuth2AuthorizationCodeFlow& google, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateEventForm),
    event_(event),
    google_(&google)
{
    ui->setupUi(this);

    ui->titleEdit->setText(event_->description());
    ui->locationEdit->setText(event_->location());
    ui->startDateTime->setDateTime(event_->getStartDateTime());
    ui->endDateTime->setDateTime(event_->getEndDateTime());

    connect(ui->titleEdit, &QLineEdit::textChanged, [this](const QString& text) { event_->setName(text); });
    connect(ui->locationEdit, &QLineEdit::textChanged, [this](const QString& text) { event_->setLocation(text); } );
    connect(ui->descriptionEdit, &QTextEdit::textChanged, [this]() { event_->setDescription(ui->descriptionEdit->toHtml()); });

    connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setStartDateTime(datetime); });
    connect(ui->endDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setEndDateTime(datetime); });

    connect(ui->saveButton, &QPushButton::clicked, [this]{ CalendarClient_CalDAV::saveEvent(*google_, event_); accept(); });
    connect(ui->deleteButton, &QPushButton::clicked, [this] { qDebug() << event_->ToICalendarObject(); delete event_; accept(); } );
}

CreateEventForm::~CreateEventForm()
{
    delete ui;
}

void CreateEventForm::on_allDayBox_stateChanged(int arg1)
{
  if(arg1 == Qt::CheckState::Checked)
  {

  }
  else
  {

  }
}

