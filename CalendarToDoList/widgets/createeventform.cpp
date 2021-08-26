#include "createeventform.h"

#include "CalendarClient/calendarclient.h"
#include "ui_createeventform.h"

CreateEventForm::CreateEventForm(CalendarEvent* event, CalendarClient& client,
                                 Calendar& calendar, bool existing, bool isEvent,
                                 QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateEventForm),
      event_(event),
      client_(&client),
      calendar_(&calendar),
<<<<<<< HEAD
      existing_(existing) {
=======
      existing_(existing),
      isEvent_(isEvent) {

>>>>>>> 365941f8c31110729955d4a5a0772d96c5b6b4d8
  ui->setupUi(this);

  ResetFormFields();

  if (existing_) ui->typeSelection->hide();

  // TYPE
  if(isEvent){
      ui->typeSelection->setCurrentText("Evento");
  }
  else{
      ui->typeSelection->setCurrentText("Attività");
  }
  connect(ui->typeSelection, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            if (text == "Evento") {
              event_ = new CalendarEvent;
              isEvent_ = true;
            } else {
              event_ = new Task;
            if (text == "Evento"){
              if(!existing_){
                  event_ = new CalendarEvent;
              }
              //isEvent_ = true;
            }
            else{
              if(!existing_){
                  event_ = new Task;
              }
              //isEvent_ = false;
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
            QDateTime max = ui->endDateTime->dateTime();
            if (datetime > max) {
              ui->startDateTime->setDateTime(max);
              return;
            }
            event_->setStartDateTime(datetime);
          });
  // EDATETIME
  connect(ui->endDateTime, &QDateTimeEdit::dateTimeChanged,
          [this](const QDateTime& datetime) {
            QDateTime min = ui->startDateTime->dateTime();
            if (datetime < min) {
              ui->endDateTime->setDateTime(min);
              return;
            }
            event_->setEndDateTime(datetime);
          });
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
  // TASKLISTS
  auto reply = client_->getAllTaskLists();
  connect(reply, &QNetworkReply::finished, [this, reply]() {
    QJsonDocument json = QJsonDocument().fromJson(reply->readAll());
    QJsonArray task_lists = json["items"].toArray();
    for (const auto& json_obj : task_lists)
      ui->taskLists->addItem(json_obj.toObject()["title"].toString());
  });

  connect(ui->saveButton, &QPushButton::clicked, [this] {
    if (!existing_) {
<<<<<<< HEAD
      if (isEvent_) {
        client_->saveEvent(*event_);
        calendar_->events().append(*event_);
        qDebug() << "New event saved\n";
      } else {
        QString title = ui->taskLists->currentText();
        for (TaskList& list : calendar_->taskLists()) {
          if (list.title() == title) {
            Task* task = qobject_cast<Task*>(event_);
            auto reply = client_->createTask(list, *task);

            connect(reply, &QNetworkReply::finished, [reply]() {
              qDebug() << reply
                              ->attribute(
                                  QNetworkRequest::HttpStatusCodeAttribute)
                              .toString();
            });

            list.getTasks().append(*task);
            break;
          }
=======
        if(isEvent_){
            client_->saveEvent(*event_);
            calendar_->events().append(*event_);
            qDebug() << "New event saved\n";
        }
        else{
            QString title = ui->taskLists->currentText();
            for(TaskList& list : calendar_->taskLists()){
                if(list.title() == title){
                    Task *task = qobject_cast<Task *>(event_);
                    auto reply = client_->createTask(list, *task);
                    list.getTasks().append(*task);
                    break;
                }
            }
            qDebug() << "New task saved\n";
>>>>>>> 365941f8c31110729955d4a5a0772d96c5b6b4d8
        }
        qDebug() << "New task saved\n";
      }
    } else {
      if (isEvent_) {
        QString hrefToUpdate = event_->getHREF();
        qDebug() << "\n\nETag: " +
                        client_->getETags().find(hrefToUpdate).value() + "\n\n";
        client_->updateEvent(*event_,
                             client_->getETags().find(hrefToUpdate).value());
        for (CalendarEvent& ev : calendar_->events()) {
          if (ev.getHREF() == hrefToUpdate) {
            calendar_->events().removeOne(ev);
          }
        }
<<<<<<< HEAD
        calendar_->events().append(*event_);
        qDebug() << "Event " + event_->summary() + " updated\n";
      } else {
        QString title = ui->taskLists->currentText();
        for (TaskList& list : calendar_->taskLists()) {
          if (list.title() == title) {
            for (Task t : list.getTasks()) {
              if (t.getHREF() == event_->getHREF()) {
                list.getTasks().removeOne(t);
              }
=======
        else{
            QString title = ui->taskLists->currentText();
            for(TaskList& list : calendar_->taskLists()){
                if(list.title() == title){
                    for(Task& t : list.getTasks()){
                        if(t.getHREF() == event_->getHREF()){
                            list.getTasks().removeOne(t);
                        }
                    }
                    Task *task = qobject_cast<Task *>(event_);
                    auto reply = client_->updateTask(list, *task);

                    connect(reply, &QNetworkReply::finished, [reply](){
                       qDebug() << reply->readAll();
                    });

                    list.getTasks().append(*task);
                    qDebug() << "Task " + task->summary() + " saved\n";
                    break;
                }
>>>>>>> 365941f8c31110729955d4a5a0772d96c5b6b4d8
            }
            Task* task = qobject_cast<Task*>(event_);
            client_->updateTask(list, *task);
            list.getTasks().append(*task);
            qDebug() << "Task " + task->summary() + " saved\n";
            break;
          }
        }
      }
    }
    emit requestView();
    accept();
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
        client_->deleteEvent(*event_, eTag);
        qDebug() << "Event " + event_->summary() + " deleted\n";
      } else {
        QString title = ui->taskLists->currentText();
        for (TaskList& list : calendar_->taskLists()) {
          if (list.title() == title) {
            Task* task = qobject_cast<Task*>(event_);
            client_->deleteTask(list, *task);
            list.getTasks().removeOne(*task);
            qDebug() << "Task " + (*task).summary() + " deleted\n";
            break;
          }
        }
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
    ui->taskLists->show();
    ui->completionButton->show();
    // Completion button
    QString text = task->completed().first ? "Segna come non completata"
                                           : "Segna come completata";
    ui->completionButton->setText(text);
  } else {
    ui->taskLists->hide();
    ui->completionButton->hide();
    // Show
    ui->RRule->show();
    ui->locationEdit->show();
    ui->startDateTime->show();
  }
}
