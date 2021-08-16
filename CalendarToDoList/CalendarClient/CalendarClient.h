#ifndef CALENDARCLIENT_CALDAV_H
#define CALENDARCLIENT_CALDAV_H

#include <QObject>
#include <QDomDocument>

#include "googleauth/googleauth.h"
#include "calendar_classes/calendarevent.h"

class CalendarClient: public QObject
{
  Q_OBJECT

public:
  Q_PROPERTY(QDomElement cTag_ READ getCTag WRITE setCTag)

  CalendarClient(GoogleAuth& auth, QObject* parent = NULL);

  ~CalendarClient();

  /**
   * @brief Obtain the ctag from the server.
   */
  QNetworkReply* obtainCTag();

  const QDomElement &getCTag() { return cTag_ ;};
  void setCTag(const QDomElement &new_cTag) { cTag_ = new_cTag; };

  QMap<QString, QDomElement> getETags(){ return eTags_; }
  void addETag(QString UID, const QDomElement &new_eTag) { eTags_.insert(UID, new_eTag); };

  QList<QString> getChangedUIDs(){ return changedUIDs_; }
  void addChangedUID(QString new_UID) { changedUIDs_.append(new_UID); };


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
   * @brief Updates a event in the calDAV server.
   */
  void updateEvent(CalendarEvent event, QString eTag);
  /**
   * @brief Saves a event to the calDAV server.
   *
   * If the uid parameter is empty, a new event will be created.
   */
  void saveEvent(CalendarEvent event);

  /**
   * @brief Deletes a specific event from the calDAV server.
   */
  void deleteEvent(QString href);

private:
  GoogleAuth* auth_;
  QDomElement cTag_;
  QMap<QString, QDomElement> eTags_;
  QList<QString> changedUIDs_;

  QNetworkAccessManager* network_manager_;
  QNetworkReply* upload_reply_;
  QTimer upload_request_timeout_timer_;

};

#endif // CALENDARCLIENT_CALDAV_H
