#include "CalendarClient.h"

#include <QByteArray>
#include <QDomDocument>
#include <QFile>
#include <QMap>
#include <QString>

#include "calendar_classes/calendar.h"

#define TASKLISTS_REQUEST_URL \
  "https://tasks.googleapis.com/tasks/v1/users/@me/lists"
#define TASKS_REQUEST_URL "https://tasks.googleapis.com/tasks/v1/lists"
#define API_KEY "AIzaSyBvVO3Q4_lqaXN6PozWeQK2CIr6fIQ7Z5w"

CalendarClient::CalendarClient(QObject* parent)
    : network_manager_(QNetworkAccessManager()) {
  QFile auth_file("auth.json");
  if (!auth_file.exists()) QT_THROW("Missing file auth.json");
  QJsonObject json = QJsonDocument().fromJson(auth_file.readAll()).object();

  endpoint_ = QUrl(json["url"].toString());
  credentials_ =
      (json["username"].toString() + ":" + json["password"].toString())
          .toUtf8()
          .toBase64();
}

//////////// Events APIs ////////////

QNetworkReply* CalendarClient::obtainCTag() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", "Basic " + credentials_);
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "0");

  QDomDocument xml;
  QDomElement root = xml.createElement("d:propfind");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:cs", endpoint_.toString());
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:displayname"));
  tagProp.appendChild(xml.createElement("cs:getctag"));
  root.appendChild(tagProp);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getAllEvents() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "1");

  QDomDocument xml;
  QDomElement root = xml.createElement("c:calendar-query");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  tagProp.appendChild(xml.createElement("c:calendar-data"));
  root.appendChild(tagProp);
  QDomElement tagFilter = xml.createElement("c:filter");
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::lookForChanges() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "1");

  QDomDocument xml;
  QDomElement root = xml.createElement("c:calendar-query");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  root.appendChild(tagProp);
  QDomElement tagFilter = xml.createElement("c:filter");
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getChangedEvents() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "1");

  QDomDocument xml;
  QDomElement root = xml.createElement("c:calendar-multiget");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  tagProp.appendChild(xml.createElement("c:calendar-data"));
  root.appendChild(tagProp);
  QDomElement tagHref;
  for (QString Item : changedItems_) {
    tagHref = xml.createElement("d:href");
    tagHref.appendChild(xml.createTextNode(Item));
    root.appendChild(tagHref);
  }

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

void CalendarClient::getDateRangeEvents(QDateTime start, QDateTime end) {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, 0);
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "1");

  QDomDocument xml;
  QDomElement root = xml.createElement("c:calendar-query");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  tagProp.appendChild(xml.createElement("c:calendar-data"));
  root.appendChild(tagProp);
  QDomElement tagFilter = xml.createElement("c:filter");
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter);
  QDomElement tagTime = xml.createElement("c:time-range");
  tagTime.setAttribute("start", start.toUTC().toString());
  tagTime.setAttribute("end", end.toUTC().toString());
  tagCompFilter.appendChild(tagTime);
  root.appendChild(tagFilter);

  auto reply = network_manager_.sendCustomRequest(
      cal_part, QByteArray("REPORT"), xml.toByteArray());

  connect(reply, &QNetworkReply::finished,
          [reply]() { qDebug() << reply->readAll(); });
}

void CalendarClient::saveEvent(CalendarEvent& event) {
  qDebug() << "saving new event:\n\n" << event.ToICalendarObject();

  if (event.getUID().isEmpty()) {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") +
                 "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + event.ToICalendarObject() + "END:VCALENDAR\r\n")
          .toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_.toString() + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  // QSslConfiguration conf = request.sslConfiguration();
  // conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  // request.setSslConfiguration(conf);

  // Bisogna ottenere il .ics corretto
  // (/home/lisa/calendars/events/qwue23489.ics)
  auto reply = network_manager_.sendCustomRequest(cal_part, QByteArray("PUT"),
                                                  request_string);

  qDebug() << "Put request sent\n";
}

void CalendarClient::updateEvent(CalendarEvent event, QString eTag) {
  qDebug() << "updating an existing event: " << event.getUID();

  if (event.getUID().isEmpty()) {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") +
                 "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + event.ToICalendarObject() + "END:VCALENDAR\r\n")
          .toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_.toString() + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  auto reply = network_manager_.sendCustomRequest(cal_part, QByteArray("PUT"),
                                                  request_string);

  qDebug() << "Put request for update sent\n";
  qDebug() << reply->readAll();

  connect(reply, &QNetworkReply::finished, [reply]() {
    qDebug() << reply->attribute(
        QNetworkRequest::Attribute::HttpStatusCodeAttribute);
  });
}

void CalendarClient::deleteEvent(CalendarEvent& event, QString eTag) {
  if (eTag.isEmpty()) {
    return;
  }

  qDebug() << "deleting event with eTag" << eTag;

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_.toString() + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);

  auto reply = network_manager_.deleteResource(cal_part);

  connect(reply, &QNetworkReply::finished, []() {

  });
}

//////////// Tasks APIs ////////////

QNetworkReply* CalendarClient::getAllTaskLists() {
  QNetworkRequest cal_part;
  cal_part.setUrl(
      QUrl(QString(TASKLISTS_REQUEST_URL) + "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("GET"));
}

QNetworkReply* CalendarClient::createTaskList(TaskList& listToCreate) {
  QNetworkRequest cal_part;
  cal_part.setUrl(
      QUrl(QString(TASKLISTS_REQUEST_URL) + "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/json");

  QJsonDocument doc(listToCreate.ToJson());
  QByteArray body = doc.toJson();

  return network_manager_.sendCustomRequest(cal_part, QByteArray("POST"), body);
}

QNetworkReply* CalendarClient::updateTaskList(TaskList& listToUpdate) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKLISTS_REQUEST_URL) + "/" +
                       listToUpdate.id() + "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/json");

  QJsonDocument doc(listToUpdate.ToJson());
  QByteArray body = doc.toJson();

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PATCH"),
                                            body);
}

QNetworkReply* CalendarClient::deleteTaskList(TaskList& listToDelete) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKLISTS_REQUEST_URL) + "/" +
                       listToDelete.id() + "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/json");

  QJsonDocument doc(listToDelete.ToJson());
  QByteArray body = doc.toJson();

  return network_manager_.sendCustomRequest(cal_part, QByteArray("DELETE"),
                                            body);
}

QNetworkReply* CalendarClient::getAllTasks(TaskList& list) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKS_REQUEST_URL) + "/" + list.id() +
                       "/tasks?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("GET"));
}

QNetworkReply* CalendarClient::getTask(TaskList& list, Task& taskToGet) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKS_REQUEST_URL) + "/" + list.id() +
                       "/tasks/" + taskToGet.getHREF() +
                       "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("GET"));
}

QNetworkReply* CalendarClient::createTask(TaskList& list, Task& newTask) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKS_REQUEST_URL) + "/" + list.id() +
                       "/tasks?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/json");

  QString newDate = newTask.getEndDateTime().toString(Qt::ISODate);
  QJsonObject obj = newTask.ToJson();
  obj["due"] = "";
  QJsonDocument doc(obj);
  qDebug() << doc;

  QByteArray body = doc.toJson();

  // qDebug()<< body;

  return network_manager_.sendCustomRequest(cal_part, QByteArray("POST"), body);
}

QNetworkReply* CalendarClient::updateTask(TaskList& list, Task& taskToUpdate) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKS_REQUEST_URL) + "/" + list.id() +
                       "/tasks/" + taskToUpdate.getHREF() +
                       "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/json");

  QJsonObject obj = taskToUpdate.ToJson();
  obj["due"] = "";
  QJsonDocument doc(obj);
  QByteArray body = doc.toJson();

  qDebug() << body;

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PATCH"),
                                            body);
}

QNetworkReply* CalendarClient::deleteTask(TaskList& list, Task& taskToDelete) {
  QNetworkRequest cal_part;
  cal_part.setUrl(QUrl(QString(TASKS_REQUEST_URL) + "/" + list.id() +
                       "/tasks/" + taskToDelete.getHREF() +
                       "?key=" + QString(API_KEY)));
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setRawHeader("Accept", "application/json");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("DELETE"));
}
