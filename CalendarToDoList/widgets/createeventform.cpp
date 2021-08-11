#include "createeventform.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateEventForm),
    event_(event)
{
    ui->setupUi(this);

    ui->titleEdit->setText(event_->description());
    ui->locationEdit->setText(event_->location());
    ui->startDateTime->setDateTime(event_->getStartDateTime());
    ui->endDateTime->setDateTime(event_->getEndDateTime());

    connect(ui->titleEdit, &QLineEdit::textChanged, [this](const QString& text) {
        event_->setDescription(text);
        qDebug() << event_->ToICalendarObject();
    });
    connect(ui->locationEdit, &QLineEdit::textChanged, [this](const QString& text) { event_->setLocation(text); } );

    connect(ui->startDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setStartDateTime(datetime); });
    connect(ui->endDateTime, &QDateTimeEdit::dateTimeChanged, [this](const QDateTime& datetime) { event_->setEndDateTime(datetime); });

    connect(ui->deleteButton, &QPushButton::clicked, [=]{ delete event_; accept(); } );
}

CreateEventForm::~CreateEventForm()
{
    delete ui;
}
