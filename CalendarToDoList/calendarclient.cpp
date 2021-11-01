#include "calendarclient.h"

#include <QByteArray>
#include <QDomDocument>
#include <QFile>
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QString>

#include "calendar_classes/calendar.h"
#include "errormanager.h"

CalendarClient::CalendarClient() {}

//////////// Events APIs ////////////

QNetworkReply* CalendarClient::obtainCTag() {
  if (!supportedMethods_.contains("PROPFIND")) {
    ErrorManager::supportError(nullptr, "Method PROPFIND not supported");
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
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(host_));
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
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(principal_);
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
  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(userCalendars_);
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
    ErrorManager::supportError(nullptr, "Method PROPFIND not supported");
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

QNetworkReply* CalendarClient::getElementByUID(QString UID, bool isEvent) {
  if (!supportedMethods_.contains("REPORT")) {
    ErrorManager::supportError(nullptr, "Method REPORT not supported");
  }

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(endpoint_));
  cal_part.setRawHeader("Depth", "1");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "application/xml; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader,
                     "xxxx");
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
  QDomElement tagCompFilter1 = xml.createElement("c:comp-filter");
  tagCompFilter1.setAttribute("name", "VCALENDAR");
  tagFilter.appendChild(tagCompFilter1);
  QDomElement tagCompFilter2 = xml.createElement("c:comp-filter");
  tagCompFilter2.setAttribute("name", isEvent ? "VEVENT" : "VTODO");
  tagCompFilter1.appendChild(tagCompFilter2);
  QDomElement tagPropFilter = xml.createElement("c:prop-filter");
  tagPropFilter.setAttribute("name", "UID");
  tagCompFilter2.appendChild(tagPropFilter);
  QDomElement tagTextMatch = xml.createElement("c:text-match");
  tagPropFilter.setAttribute("collation", "i;octet");
  tagTextMatch.appendChild(xml.createTextNode(UID));
  tagPropFilter.appendChild(tagTextMatch);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::getDateRangeEvents(QDateTime start,
                                                  QDateTime end) {
  if (!supportedMethods_.contains("REPORT")) {
    ErrorManager::supportError(nullptr, "Method REPORT not supported");
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
  // QDomElement expand = xml.createElement("c:expand");
  // expand.setAttribute("start",
  // start.toUTC().toString("yyyyMMdd'T'hhmmss'Z'")); expand.setAttribute("end",
  // end.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  // cal_data.appendChild(expand);
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

QNetworkReply* CalendarClient::receiveChangesBySyncToken() {
  if (!supportedMethods_.contains("REPORT")) {
    ErrorManager::supportError(nullptr, "Method REPORT not supported");
  }
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
  tagProp.appendChild(calendar_data);
  root.appendChild(tagProp);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}

QNetworkReply* CalendarClient::saveElement(ICalendarComponent& newElement) {
  if (!supportedMethods_.contains("PUT")) {
    ErrorManager::supportError(nullptr, "Method PUT not supported");
  }

  QByteArray request_string =
      ("BEGIN:VCALENDAR\r\n" + newElement.toICalendar() + "END:VCALENDAR\r\n")
          .toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(
      QUrl(endpoint_.toString() + "/" + newElement.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                     "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::IfNoneMatchHeader, "*");

  return network_manager_.sendCustomRequest(cal_part, QByteArray("PUT"),
                                            request_string);
}

QNetworkReply* CalendarClient::updateElement(ICalendarComponent& updatedElement,
                                             QString eTag) {
  if (!supportedMethods_.contains("PUT")) {
    ErrorManager::supportError(nullptr, "Method PUT not supported");
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

QNetworkReply* CalendarClient::deleteElement(ICalendarComponent& event,
                                             QString eTag) {
  if (!supportedMethods_.contains("DELETE")) {
    ErrorManager::supportError(nullptr, "Method DELETE not supported");
  }
  if (eTag.isEmpty()) {
    return nullptr;
  }

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Basic " + credentials_));
  cal_part.setUrl(QUrl(host_.toString() + event.href()));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                     "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("DELETE"));
}

QNetworkReply* CalendarClient::getExpandedRecurrentEvent(
    const QString& uid, QPair<QDateTime, QDateTime> range) {
  if (!supportedMethods_.contains("REPORT")) {
    ErrorManager::supportError(nullptr, "Method REPORT not supported");
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
  expand.setAttribute("start",
                      range.first.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  expand.setAttribute("end",
                      range.second.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  cal_data.appendChild(expand);
  tagProp.appendChild(cal_data);
  root.appendChild(tagProp);
  // FILTER
  QDomElement tagFilter = xml.createElement("c:filter");
  // Filter VCALENDAR
  QDomElement tagCompFilter = xml.createElement("c:comp-filter");
  tagCompFilter.setAttribute("name", "VCALENDAR");
  // Filter VEVENT
  QDomElement tag_comp_filter2 = xml.createElement("c:comp-filter");
  tag_comp_filter2.setAttribute("name", "VEVENT");
  // Time range filter
  QDomElement tagTime = xml.createElement("c:time-range");
  tagTime.setAttribute("start",
                       range.first.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  tagTime.setAttribute("end",
                       range.second.toUTC().toString("yyyyMMdd'T'hhmmss'Z'"));
  // UID match
  QDomElement tagPropFilter = xml.createElement("c:prop-filter");
  tagPropFilter.setAttribute("name", "UID");
  tagCompFilter.appendChild(tagPropFilter);
  QDomElement tagTextMatch = xml.createElement("c:text-match");
  tagPropFilter.setAttribute("collation", "i;octet");
  tagTextMatch.appendChild(xml.createTextNode(uid));
  tagPropFilter.appendChild(tagTextMatch);

  tagPropFilter.appendChild(tagTextMatch);
  tag_comp_filter2.appendChild(tagPropFilter);
  tagCompFilter.appendChild(tag_comp_filter2);
  tagFilter.appendChild(tagCompFilter);
  root.appendChild(tagFilter);

  return network_manager_.sendCustomRequest(cal_part, QByteArray("REPORT"),
                                            xml.toByteArray());
}
