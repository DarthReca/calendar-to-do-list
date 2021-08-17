#include "createeventform.h"
#include "ui_createeventform.h"
#include "CalendarClient/calendarclient.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateEventForm),
    event_(event),
    client_(&client)
{
    ui->setupUi(this);

    ui->titleEdit->setText(event_->name());
    ui->locationEdit->setText(event_->location());
    ui->startDateTime->setDateTime(event_->getStartDateTime());
    ui->endDateTime->setDateTime(event_->getEndDateTime());
    ui->RRule->setPlaceholderText("Ripeti:");
    ui->RRule->addItem("Non si ripete");
    ui->RRule->addItem("Ogni giorno");
    ui->RRule->addItem("Ogni settimana");
    ui->RRule->addItem("Ogni mese");
    ui->RRule->addItem("Ogni anno");


    connect(ui->titleEdit, &QLineEdit::textChanged, [this](const QString& text) { event_->setName(text); });
    connect(ui->locationEdit, &QLineEdit::textChanged, [this](const QString& text) { event_->setLocation(text); } );
    connect(ui->descriptionEdit, &QTextEdit::textChanged, [this]() { event_->setDescription(ui->descriptionEdit->toHtml()); });

    connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setStartDateTime(datetime); });
    connect(ui->endDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setEndDateTime(datetime); });

    connect(ui->RRule, &QComboBox::currentTextChanged, [this](const QString& text) {
        if(text=="Non si ripete"){
            event_->setRRULE("");
        }
        if(text=="Ogni giorno"){
            event_->setRRULE("FREQ=DAILY");
        }
        if(text=="Ogni settimana"){
            event_->setRRULE("FREQ=WEEKLY");
        }
        if(text=="Ogni mese"){
            event_->setRRULE("FREQ=MONTHLY");
        }
        if(text=="Ogni anno"){
            event_->setRRULE("FREQ=YEARLY");
        }
    });

    connect(ui->saveButton, &QPushButton::clicked, [this]{ client_->saveEvent(*event_); accept(); });
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
