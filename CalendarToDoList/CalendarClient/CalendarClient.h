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
#include "calendar_classes/icalendarcomponent.h"
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

  QSet<QString>& getSupportedMethods() { return supportedMethods_; };
  void setSupportedMethods(const QSet<QString>& new_supportedMethods) {
    supportedMethods_ = new_supportedMethods;
  };

  const QString& getSyncToken() { return syncToken_; };
  void setSyncToken(const QString& new_syncToken) {
    syncToken_ = new_syncToken;
  };

  QMap<QString, QString>& eTags() { return eTags_; }
  void addETag(QString href, const QString& new_eTag) {
    eTags_.insert(href, new_eTag);
  };
  void deleteETag(QString href) { eTags_.remove(href); };

  void setHost(QUrl& host) { host_ = host; };
  QUrl getHost() { return host_; };
  void setCredentials(QByteArray& credentials) { credentials_ = credentials; };
  QString getCredentials() { return credentials_; };
  void setEndpoint(QUrl& endpoint) { endpoint_ = endpoint; };
  QUrl getEndpoint() { return endpoint_; };


  QList<QString> getChangedItems() { return changedItems_; }
  void addChangedItem(QString new_Item) { changedItems_.append(new_Item); }
  void deleteChangedItem(QString href) { changedItems_.removeOne(href); };
  void clearChangedItems() { changedItems_.clear(); }

 public slots:

  //////////// Events/Tasks APIs ////////////
  /**
   * @brief Discovers the principal user.
   */
  QNetworkReply* discoverUser();
  /**
   * @brief Discovers the principal user's calendars.
   */
  QNetworkReply* discoverUserCalendars();
  /**
   * @brief Lists the principal user's calendars.
   */
  QNetworkReply* listUserCalendars();
  /**
   * @brief Finds out which operations the server supports.
   */
  QNetworkReply* findOutCalendarSupport();
  /**
   * @brief Finds out the properties returned.
   */
  QNetworkReply* findOutSupportedProperties();
  /**
   * @brief Gets all elements from the calDAV server.
   */
  QNetworkReply* getAllElements();
  /**
   * @brief Gets an event with a specific UID from the calDAV server.
   */
  QNetworkReply* getElementByUID(QString UID, bool isEvent);
  /**
   * @brief Gets all events in a specific time range.
   */
  QNetworkReply* getDateRangeEvents(QDateTime start, QDateTime end);
  /**
   * @brief Gets all events in a specific time range.
   */
  QNetworkReply* getDateRangeTasks(QDateTime start, QDateTime end);
  /**
   * @brief Requests the sync token from the server.
   */
  QNetworkReply* requestSyncToken();
  /**
   * @brief Receives the changed elements through the sync token.
   */
  QNetworkReply* receiveChangesBySyncToken();
  /**
   * @brief Looks for changes done in the server.
   */
  QNetworkReply* lookForChanges();
  /**
   * @brief Fetches the changed things in the calendar.
   */
  QNetworkReply* getChangedElements();
  /**
   * @brief Saves a event to the calDAV server.
   *
   * If the uid parameter is empty, a new event will be created.
   */
  QNetworkReply* saveElement(ICalendarComponent& event);
  /**
   * @brief Updates a event in the calDAV server.
   */
  QNetworkReply* updateElement(ICalendarComponent& updatedElement,
                               QString eTag);
  /**
   * @brief Deletes a specific event from the calDAV server.
   */
  QNetworkReply* deleteElement(ICalendarComponent& event, QString eTag);

  QNetworkReply* getExpandedRecurrentEvent(const QString& href,
                                           QPair<QDateTime, QDateTime> range);

 private:
  QString cTag_;
  QString syncToken_;
  QMap<QString, QString> eTags_;
  QList<QString> changedItems_;
  QSet<QString> supportedMethods_;

  QNetworkAccessManager network_manager_;
  QByteArray credentials_;

  QUrl endpoint_;
  QUrl host_;
};

#endif  // CALENDARCLIENT_CALDAV_H
