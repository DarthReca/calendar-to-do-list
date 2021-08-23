#include "calendarclient.h"

#include <QByteArray>
#include <QDomDocument>
#include <QMap>
#include <QString>

#include "calendar_classes/calendar.h"

#define EVENTS_REQUEST_URL                                  \
  "https://apidata.googleusercontent.com/caldav/v2/" \
  "k8tsgo0usjdlipul5pb2vel68o@group.calendar.google.com/events"

#define TASKLISTS_REQUEST_URL "https://tasks.googleapis.com/tasks/v1/users/@me/lists"
#define TASKS_REQUEST_URL "https://tasks.googleapis.com/tasks/v1/lists"

CalendarClient::CalendarClient(GoogleAuth& auth, QObject* parent) {
  auth_ = &auth;
}

CalendarClient::~CalendarClient() { delete auth_; }



//////////// Events APIs ////////////



QNetworkReply* CalendarClient::obtainCTag() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(EVENTS_REQUEST_URL));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "0");

  QDomDocument xml;
  QDomElement root = xml.createElement("d:propfind");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:cs", EVENTS_REQUEST_URL);
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:displayname"));
  tagProp.appendChild(xml.createElement("cs:getctag"));
  root.appendChild(tagProp);

  return auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply* CalendarClient::getAllEvents() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(EVENTS_REQUEST_URL));
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

  return auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("REPORT"), xml.toByteArray());
}

QNetworkReply* CalendarClient::lookForChanges() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(EVENTS_REQUEST_URL));
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

  return auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("REPORT"), xml.toByteArray());
}

QNetworkReply* CalendarClient::getChangedEvents() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(EVENTS_REQUEST_URL));
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

  return auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("REPORT"), xml.toByteArray());
}

void CalendarClient::getDateRangeEvents(QDateTime start, QDateTime end) {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(EVENTS_REQUEST_URL));
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

  auto reply = auth_->google->networkAccessManager()->sendCustomRequest(
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
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(EVENTS_REQUEST_URL) + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  // QSslConfiguration conf = request.sslConfiguration();
  // conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  // request.setSslConfiguration(conf);

  // Bisogna ottenere il .ics corretto
  // (/home/lisa/calendars/events/qwue23489.ics)
  auto reply = auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("PUT"), request_string);

  qDebug() << "Put request sent\n";
  connect(reply, &QNetworkReply::finished, [reply]() {
    for(int i=0; i<reply->rawHeaderPairs().size(); i++)
        qDebug() << "\n\nHeader: " + reply->rawHeaderPairs().at(i).first + reply->rawHeaderPairs().at(i).second + "\n\n";

      qDebug() << "\n\nBody: " + reply->readAll() + "\n\n";
  });
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
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(EVENTS_REQUEST_URL) + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  auto reply = auth_->google->networkAccessManager()->sendCustomRequest(
      cal_part, QByteArray("PUT"), request_string);

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
  cal_part.setRawHeader("Authorization",
                        ("Bearer " + auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(EVENTS_REQUEST_URL) + "/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);

  auto reply = auth_->google->networkAccessManager()->deleteResource(cal_part);

  connect(reply, &QNetworkReply::finished, []() {

  });
}



//////////// Tasks APIs ////////////



QNetworkReply *CalendarClient::getAllTaskLists()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer " + auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(QString(TASKLISTS_REQUEST_URL)));
    cal_part.setRawHeader("Accept", "application/json");

    return auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("GET"));
}
