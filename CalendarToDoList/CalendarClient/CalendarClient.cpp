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
  auth_file.open(QFile::OpenModeFlag::ReadOnly);
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

QNetworkReply *CalendarClient::discoverUser()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl("/"));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                       "CalendarClient_CalDAV");
    cal_part.setRawHeader("Depth", "0");
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                       "application/xml; charset=utf-8");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:current-user-principal"));
    root.appendChild(tagProp);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply *CalendarClient::discoverUserCalendars()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                       "CalendarClient_CalDAV");
    cal_part.setRawHeader("Depth", "0");
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                       "application/xml; charset=utf-8");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("c:calendar-home-set"));
    root.appendChild(tagProp);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply *CalendarClient::listUserCalendars()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                       "CalendarClient_CalDAV");
    cal_part.setRawHeader("Depth", "1");
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                       "application/xml; charset=utf-8");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    root.setAttribute("xmlns:cs", "http://calendarserver.org/ns/");
    root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:resourcetype"));
    tagProp.appendChild(xml.createElement("d:displayname"));
    tagProp.appendChild(xml.createElement("cs:getctag"));
    tagProp.appendChild(xml.createElement("c:supported-calendar-component-set"));
    root.appendChild(tagProp);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply *CalendarClient::findOutCalendarSupport()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                       "CalendarClient_CalDAV");
    cal_part.setRawHeader("Host", "cal.example.com");

    return network_manager_.sendCustomRequest(cal_part, QByteArray("OPTIONS"));
}

QNetworkReply *CalendarClient::findOutSupportedProperties()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                       "CalendarClient_CalDAV");
    cal_part.setRawHeader("Host", "www.example.com");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                       "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader,
                       "xxxx");

    QDomDocument xml;
    QDomElement root = xml.createElement("propfind");
    root.setAttribute("xmlns", "DAV:");
    root.appendChild(xml.createElement("propname"));
    xml.appendChild(root);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                              xml.toByteArray());
}

QNetworkReply* CalendarClient::getAllElements() {
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

QNetworkReply* CalendarClient::getChangedElements() {
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

QNetworkReply *CalendarClient::requestSyncToken()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
    cal_part.setRawHeader("Depth", "0");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    root.setAttribute("xmlns:cs", endpoint_.toString());
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:displayname"));
    tagProp.appendChild(xml.createElement("cs:getctag"));
    tagProp.appendChild(xml.createElement("d:sync-token"));
    root.appendChild(tagProp);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply *CalendarClient::receiveChangesBySyncToken()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
    cal_part.setUrl(QUrl(endpoint_));
    cal_part.setRawHeader("Host", "dav.example.org");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:sync-collection");
    root.setAttribute("xmlns:d", "DAV:");
    xml.appendChild(root);
    QDomElement tagToken = xml.createElement("d:sync-token");
    tagToken.appendChild(xml.createTextNode(syncToken_));
    root.appendChild(tagToken);
    QDomElement tagLevel = xml.createElement("d:sync-level");
    tagLevel.appendChild(xml.createTextNode("1"));
    root.appendChild(tagLevel);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:getetag"));
    root.appendChild(tagProp);

    return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());
}

void CalendarClient::saveElement(CalendarEvent& event) {
  qDebug() << "saving new event:\n\n" << event.ToVEvent();

  if (event.getUID().isEmpty()) {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") +
                 "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + event.ToVEvent() + "END:VCALENDAR\r\n")
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

void CalendarClient::updateElement(CalendarEvent event, QString eTag) {
  qDebug() << "updating an existing event: " << event.getUID();

  if (event.getUID().isEmpty()) {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") +
                 "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + event.ToVEvent() + "END:VCALENDAR\r\n")
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

void CalendarClient::deleteElement(CalendarEvent& event, QString eTag) {
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
