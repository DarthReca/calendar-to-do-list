#ifndef TASK_H
#define TASK_H

#include <QColor>
#include <QJsonObject>
#include <QObject>

#include "calendarevent.h"

class Task : public CalendarEvent {
  Q_OBJECT
 public:
  Task(QObject *parent = nullptr)
      : CalendarEvent(parent),
        completed_(QPair<bool, QDateTime>(false, QDateTime())) {
    setColor(QColor(Qt::blue).name());
  };
  Task(QJsonObject &json, QObject *parent = nullptr);

  QJsonObject ToJson();
  void FlipCompleted();

  const QPair<bool, QDateTime> &completed() const;

 private:
  QPair<bool, QDateTime> completed_;
};

class TaskList {

 public:
  TaskList(const QString &title, const QString &id)
      : title_(title), id_(id){
      tasks_ = QList<Task>();
  };

  const QString &title() const { return title_; }
  void setTitle(const QString &newTitle) { title_ = newTitle; }

  const QString &id() const { return id_; }
  void setId(const QString &newId) { id_ = newId; }

  QList<Task> &getTasks() { return tasks_; }
  void setTasks(QList<Task> new_list) { tasks_ = new_list; }

  QJsonObject ToJson();

 private:
  QString title_;
  QString id_;
  QList<Task> tasks_;
};

#endif  // TASK_H
