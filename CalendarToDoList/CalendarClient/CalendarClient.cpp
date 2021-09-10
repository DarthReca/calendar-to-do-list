#include "CalendarClient.h"

#include <QByteArray>
#include <QDomDocument>
#include <QFile>
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QString>

#include "calendar_classes/calendar.h"

CalendarClient::CalendarClient(QObject* parent)
    : network_manager_(QNetworkAccessManager()) {
  QFile auth_file("auth.json");
  if (!auth_file.exists()) {
    QWidget* parent_widget = qobject_cast<QWidget*>(parent);
    QMessageBox::critical(parent_widget, "Missing file",
                          "The file auth.json is required", QMessageBox::Ok);
    exit(-1);
  }
  auth_file.open(QFile::OpenModeFlag::ReadOnly);
  QJsonObject json = QJsonDocument().fromJson(auth_file.readAll()).object();

  endpoint_ = QUrl(json["url"].toString());
  host_ = QUrl(json["host"].toString());
  credentials_ =
      (json["username"].toString() + ":" + json["password"].toString())
          .toUtf8()
          .toBase64();
}

//////////// Events APIs ////////////

QNetworkReply* CalendarClient::obtainCTag() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug() << "Method PROPFIND not supported in call obtainCTag";
    return nullptr;
  }

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
  root.setAttribute("xmlns:cs", "http://calendarserver.org/ns/");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("cs:getctag"));
  root.appendChild(tagProp);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::discoverUser() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug() << "Method PROPFIND not supported in call discoverUser";
    return nullptr;
  }

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

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::discoverUserCalendars() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug() << "Method PROPFIND not supported in call discoverUserCalendars";
    return nullptr;
  }

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

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::listUserCalendars() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug() << "Method PROPFIND not supported in call listUserCalendars";
    return nullptr;
  }

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

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::findOutCalendarSupport() {
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("OPTIONS"));
}

QNetworkReply* CalendarClient::findOutSupportedProperties() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug()
        << "Method PROPFIND not supported in call findOutSupportedProperties";
    return nullptr;
  }

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setRawHeader("Depth", "0");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");

  QDomDocument xml;
  QDomElement root = xml.createElement("d:propfind");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  root.setAttribute("xmlns:cs", "http://calendarserver.org/ns/");
  QDomElement prop = xml.createElement("d:prop");
  prop.appendChild(xml.createElement("cs:getctag"));
  prop.appendChild(xml.createElement("d:sync-token"));
  root.appendChild(prop);
  xml.appendChild(root);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getAllElements() {
  if (!supportedMethods_.contains("REPORT")) {
    qDebug() << "Method REPORT not supported in call getAllElements";
    return nullptr;
  }

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

QNetworkReply* CalendarClient::getElementByUID(QString UID) {
  if (!supportedMethods_.contains("REPORT")) {
    qDebug() << "Method REPORT not supported in call getElementByUID";
    return nullptr;
  }

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setRawHeader("Depth", "1");
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  QDomDocument xml;
  QDomElement root = xml.createElement("c:calendar-query");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.setAttribute("xmlns:d", "DAV:");
  tagProp.appendChild(xml.createElement("d:getetag"));
  tagProp.appendChild(xml.createElement("c:calendar-data"));
  root.appendChild(tagProp);
  QDomElement tagFilter = xml.createElement("c:filter");
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter);
  QDomElement tag_comp_filter2 = xml.createElement("c:comp-filter");
  tagCompFilter.appendChild(tag_comp_filter2);
  QDomElement tagPropFilter = xml.createElement("c:prop-filter");
  tagPropFilter.setAttribute("name", "UID");
  tag_comp_filter2.appendChild(tagPropFilter);
  QDomElement tagTextMatch = xml.createElement("c:text-match");
  tagPropFilter.setAttribute("collation", "i;octet");
  tagTextMatch.appendChild(xml.createTextNode(UID));
  tagPropFilter.appendChild(tagTextMatch);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::lookForChanges() {
  if (!supportedMethods_.contains("REPORT")) {
    qDebug() << "Method REPORT not supported in call lookForChanges";
    return nullptr;
  }

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
  QDomElement calendar_data = xml.createElement("c:calendar-data");
  QDomElement calendar_comp = xml.createElement("c:comp");
  calendar_comp.setAttribute("name", "VCALENDAR");
  QDomElement event_comp = xml.createElement("c:comp");
  event_comp.setAttribute("name", "VEVENT");
  QDomElement prop = xml.createElement("c:prop");
  prop.setAttribute("name", "UID");
  event_comp.appendChild(prop);
  calendar_comp.appendChild(event_comp);
  calendar_data.appendChild(calendar_comp);
  tagProp.appendChild(calendar_data);
  root.appendChild(tagProp);
  QDomElement tagFilter = xml.createElement("c:filter");
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter);
  root.appendChild(tagFilter);

  qDebug() << xml.toString();

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getChangedElements() {
  if (!supportedMethods_.contains("REPORT")) {
    qDebug() << "Method REPORT not supported in call getChangedElements";
    return nullptr;
  }

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

QNetworkReply* CalendarClient::getDateRangeEvents(QDateTime start,
                                                  QDateTime end) {
  if (!supportedMethods_.contains("REPORT")) {
    qWarning() << "Method REPORT not supported in call getDateRangeEvents";
    return nullptr;
  }

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
  // PROP
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  QDomElement cal_data = xml.createElement("c:calendar-data");
  QDomElement expand = xml.createElement("c:expand");
  expand.setAttribute("start", start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  expand.setAttribute("end", end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  cal_data.appendChild(expand);
  tagProp.appendChild(cal_data);
  root.appendChild(tagProp);
  // FILTER
  QDomElement tagFilter = xml.createElement("c:filter");
  // Filter without specefic class
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagFilter.appendChild(tagCompFilter);
  // Time range filter
  QDomElement tagTime = xml.createElement("c:time-range");
  tagTime.setAttribute("start", start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  tagTime.setAttribute("end", end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  tagFilter.appendChild(tagTime);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getDateRangeTasks(QDateTime start,
                                                 QDateTime end) {
  if (!supportedMethods_.contains("REPORT")) {
    qWarning() << "Method REPORT not supported in call getDateRangeEvents";
    return nullptr;
  }

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
  // PROP
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  QDomElement cal_data = xml.createElement("c:calendar-data");
  tagProp.appendChild(cal_data);
  root.appendChild(tagProp);
  // FILTER
  QDomElement tagFilter = xml.createElement("c:filter");
  // Filter the tasks
  QDomElement tagCompFilter1 = xml.createElement("c:comp-filter");
  tagCompFilter1.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter1);
  QDomElement tagCompFilter2 = xml.createElement("c:comp-filter");
  tagCompFilter2.setAttribute("name", "VTODO");
  tagCompFilter1.appendChild(tagCompFilter2);
  // Time range filter
  QDomElement tagTime = xml.createElement("c:time-range");
  tagTime.setAttribute("start", start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  tagTime.setAttribute("end", end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  tagFilter.appendChild(tagTime);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::requestSyncToken() {
  if (!supportedMethods_.contains("PROPFIND")) {
    qDebug() << "Method PROPFIND not supported in call requestSyncToken";
    return nullptr;
  }

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
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

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PROPFIND"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::receiveChangesBySyncToken() {
  if (!supportedMethods_.contains("REPORT")) {
    qDebug() << "Method REPORT not supported in call receiveChangesBySyncToken";
    return nullptr;
  }
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays(3);

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  QDomDocument xml;
  QDomElement root = xml.createElement("d:sync-collection");
  root.setAttribute("xmlns:d", "DAV:");
  root.setAttribute("xmlns:c", "urn:ietf:params:xml:ns:caldav");
  xml.appendChild(root);
  QDomElement tagToken = xml.createElement("d:sync-token");
  tagToken.appendChild(xml.createTextNode(syncToken_));
  root.appendChild(tagToken);
  QDomElement tagLevel = xml.createElement("d:sync-level");
  tagLevel.appendChild(xml.createTextNode("1"));
  root.appendChild(tagLevel);
  QDomElement tagProp = xml.createElement("d:prop");
  tagProp.appendChild(xml.createElement("d:getetag"));
  QDomElement calendar_data = xml.createElement("c:calendar-data");
  // Expansion of recurrencies
  QDomElement expand = xml.createElement("c:expand");
  expand.setAttribute("start", start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  expand.setAttribute("end", end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  calendar_data.appendChild(expand);
  tagProp.appendChild(calendar_data);
  root.appendChild(tagProp);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::saveElement(CalendarEvent& newElement) {
  if (!supportedMethods_.contains("PUT")) {
    qDebug() << "Method PUT not supported in call saveElement";
    return nullptr;
  }
  if (newElement.uid().isEmpty()) {
    newElement.setUid(
        QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" +
        newElement.startDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + newElement.toICalendar() + "END:VCALENDAR\r\n")
          .toUtf8();

  qDebug() << request_string;

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_.toString() + "/" + newElement.uid() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::IfNoneMatchHeader, "*");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PUT"),
                                            request_string);
}

QNetworkReply* CalendarClient::updateElement(CalendarEvent& updatedElement,
                                             QString eTag) {
  if (!supportedMethods_.contains("PUT")) {
    qDebug() << "Method PUT not supported in call updateElement";
    return nullptr;
  }
  if (updatedElement.uid().isEmpty()) {
    updatedElement.setUid(
        QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" +
        updatedElement.startDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + updatedElement.toICalendar() +
       "END:VCALENDAR\r\n")
          .toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(host_.toString() + updatedElement.href()));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PUT"),
                                            request_string);
}

QNetworkReply* CalendarClient::deleteElement(CalendarEvent& event,
                                             QString eTag) {
  if (!supportedMethods_.contains("DELETE")) {
    qDebug() << "Method DELETE not supported in call deleteElement";
    return nullptr;
  }
  if (eTag.isEmpty()) {
    qDebug() << "No eTag";
    return nullptr;
  }
  qDebug() << "deleting event with eTag" << eTag;

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(host_.toString() + event.href()));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("DELETE"));
}
