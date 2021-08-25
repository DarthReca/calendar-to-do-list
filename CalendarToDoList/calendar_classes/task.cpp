#include "task.h"

Task::Task(QJsonObject& json, QObject* parent) : Task(parent) {
  auto id = json.value("id");
  auto title = json.value("title");
  auto end_date = json.value("due");
  auto completed = json.value("completed");

  if (id != QJsonValue::Undefined) setHREF(id.toString());
  if (title != QJsonValue::Undefined) setSummary(title.toString());

  if (end_date != QJsonValue::Undefined) {
    QDateTime utcTime =
        QDateTime::fromString(end_date.toString(), Qt::ISODateWithMs);
    if (!utcTime.isValid())
      qDebug() << ": "
               << "could not parse" << end_date.toString();
    setStartDateTime(utcTime);
    setEndDateTime(utcTime);
    setAll_day(true);
  }

  completed_.first = completed != QJsonValue::Undefined;
  if (completed_.first)
    completed_.second = QDateTime().fromString(completed.toString());
}

QJsonObject Task::ToJson() {
  QJsonObject json;
  json["id"] = getHREF();
  json["title"] = summary();
  json["due"] = getEndDateTime().toString();
  if (completed_.first) json["completed"] = completed_.second.toString();
  return json;
}

void Task::FlipCompleted() {
  completed_.first = !completed_.first;
  completed_.second = QDateTime::currentDateTime();
}

const QPair<bool, QDateTime>& Task::completed() const { return completed_; }

QJsonObject TaskList::ToJson() {
  QJsonObject json;
  json["id"] = id_;
  json["title"] = title_;
  return json;
}
