#ifndef CALENDARCLIENT_CALDAV_H
#define CALENDARCLIENT_CALDAV_H

#include <QDomDocument>
#include <QObject>

#include "calendar_classes/calendarevent.h"
#include "googleauth/googleauth.h"

class CalendarClient : public QObject {
  Q_OBJECT

 public:
  CalendarClient(GoogleAuth& auth, QObject* parent = nullptr);
  ~CalendarClient();

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

  /**
   * @brief Gets all events from the calDAV server.
   */
  QNetworkReply* getAllEvents();
  /**
   * @brief Gets all events in a specific time range.
   */
  void getDateRangeEvents(QDateTime start, QDateTime end);
  /**
   * @brief Requests the token to receive changes in the server.
   */
  QNetworkReply* requestSyncToken();
  /**
   * @brief Receives the changes done in the server.
   */
  void receiveChanges(QString syncToken);
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
  void checkForChanges();

 private:
  QPointer<GoogleAuth> auth_;
  QString cTag_;
  QMap<QString, QString> eTags_;
  QList<QString> changedItems_;

  QPointer<QNetworkAccessManager> network_manager_;
};

#endif  // CALENDARCLIENT_CALDAV_H
