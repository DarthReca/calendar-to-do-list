#ifndef TASK_H
#define TASK_H

#include <QJsonObject>
#include <QObject>

#include "calendarevent.h"

class Task : public CalendarEvent {
 public:
  Task(QObject *parent = nullptr) : CalendarEvent(parent), completed_(false){};
  Task(QJsonObject &json, QObject *parent = nullptr);

  QJsonObject ToJson();

 private:
  bool completed_;
};

class TaskList : public QList<Task> {
 public:
  TaskList(const QString &title, const QString &id)
      : QList<Task>(), title_(title), id_(id){};

  const QString &title() const { return title_; }
  void setTitle(const QString &newTitle) { title_ = newTitle; }

  const QString &id() const { return id_; }
  void setId(const QString &newId) { id_ = newId; }

 private:
  QString title_;
  QString id_;
};

#endif  // TASK_H
