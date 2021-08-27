#ifndef CALENDARCLIENT_CALDAV_H
#define CALENDARCLIENT_CALDAV_H

#include <QAuthenticator>
#include <QDomDocument>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>

#include "calendar_classes/calendarevent.h"
#include "calendar_classes/task.h"

class CalendarClient : public QObject {
  Q_OBJECT

 public:
  CalendarClient(QObject* parent = nullptr);

  /**
   * @brief Obtain the ctag from the server.
   */
  QNetworkReply* obtainCTag();

  const QString& getCTag() { return cTag_; };
  void setCTag(const QString& new_cTag) { cTag_ = new_cTag; };

  QMap<QString, QString>& getETags() { return eTags_; }
  void addETag(QString href, const QString& new_eTag) {
    eTags_.insert(href, new_eTag);
  };
  void deleteETag(QString href) { eTags_.remove(href); };

  QList<QString> getChangedItems() { return changedItems_; }
  void addChangedItem(QString new_Item) { changedItems_.append(new_Item); }
  void deleteChangedItem(QString href) { changedItems_.removeOne(href); };
  void clearChangedItems() { changedItems_.clear(); }

 public slots:

  //////////// Events APIs ////////////
  /**
   * @brief Gets all events from the calDAV server.
   */
  QNetworkReply* getAllEvents();
  /**
   * @brief Gets all events in a specific time range.
   */
  void getDateRangeEvents(QDateTime start, QDateTime end);
  /**
   * @brief Looks for changes done in the server.
   */
  QNetworkReply* lookForChanges();
  /**
   * @brief Fetches the changed things in the calendar.
   */
  QNetworkReply* getChangedEvents();
  /**
   * @brief Saves a event to the calDAV server.
   *
   * If the uid parameter is empty, a new event will be created.
   */
  void saveEvent(CalendarEvent& event);
  /**
   * @brief Updates a event in the calDAV server.
   */
  void updateEvent(CalendarEvent event, QString eTag);
  /**
   * @brief Deletes a specific event from the calDAV server.
   */
  void deleteEvent(CalendarEvent& event, QString eTag);

  //////////// Tasks APIs ////////////
  /**
   * @brief Gets all taskLists from the server.
   */
  QNetworkReply* getAllTaskLists();
  /**
   * @brief Creates a new taskList in the user's taskLists.
   */
  QNetworkReply* createTaskList(TaskList& listToCreate);
  /**
   * @brief Updates an existing taskList among the user's taskLists.
   */
  QNetworkReply* updateTaskList(TaskList& listToUpdate);
  /**
   * @brief Deletes an existing taskList among the user's taskLists.
   */
  QNetworkReply* deleteTaskList(TaskList& listToDelete);
  /**
   * @brief Gets all tasks in a specified user's taskLists.
   */
  QNetworkReply* getAllTasks(TaskList& list);
  /**
   * @brief Gets a certain tasks in a specified user's taskLists.
   */
  QNetworkReply* getTask(TaskList& list, Task& taskToGet);
  /**
   * @brief Creates a new tasks in a specified user's taskLists.
   */
  QNetworkReply* createTask(TaskList& list, Task& newTask);
  /**
   * @brief Updates a certain tasks in a specified user's taskLists.
   */
  QNetworkReply* updateTask(TaskList& list, Task& taskToUpdate);
  /**
   * @brief Deletes a certain tasks in a specified user's taskLists.
   */
  QNetworkReply* deleteTask(TaskList& list, Task& taskToDelete);

 private:
  QString cTag_;
  QMap<QString, QString> eTags_;
  QList<QString> changedItems_;

  QNetworkAccessManager network_manager_;
  QByteArray credentials_;

  QUrl endpoint_;
};

#endif  // CALENDARCLIENT_CALDAV_H
