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
