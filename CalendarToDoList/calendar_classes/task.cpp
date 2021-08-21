#include "task.h"

Task::Task(QJsonObject& json, QObject* parent) {
  auto id = json.value("id");
  auto title = json.value("title");
  auto end_date = json.value("due");
  auto completed = json.value("completed");

  if (id != QJsonValue::Undefined) setHREF(id.toString());
  if (title != QJsonValue::Undefined) setSummary(title.toString());

  if (end_date != QJsonValue::Undefined) {
    setStartDateTime(QDateTime().fromString(end_date.toString()));
    setEndDateTime(QDateTime().fromString(end_date.toString()));
  }
}

QJsonObject Task::ToJson() {
  QJsonObject json;
  json["id"] = getHREF();
  json["title"] = summary();
  json["due"] = getEndDateTime().toString();
  return json;
}
