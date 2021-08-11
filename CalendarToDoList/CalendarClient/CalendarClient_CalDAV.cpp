#include <QByteArray>
#include <QString>
#include <QMap>
#include <QDomDocument>

#include "CalendarClient_CalDAV.h"

#define REQUEST_URL "https://apidata.googleusercontent.com/caldav/v2/k8tsgo0usjdlipul5pb2vel68o@group.calendar.google.com/events"

CalendarClient_CalDAV::CalendarClient_CalDAV(QObject* parent)
{
  upload_reply_ = NULL;
  m_Username = "";
  m_Password = "";
  m_syncToken = "";
  m_cTag = "";
  year_ = 1;
  month_ = 1;
  m_YearToBeRequested = QDate::currentDate().year();;
  m_MonthToBeRequested = QDate::currentDate().month();
  lastSyncYear = -1;
  lastSyncMonth = -1;
  m_bRecoveredFromError = false;
}

CalendarClient_CalDAV::~CalendarClient_CalDAV()
{
  //m_SynchronizationTimer.stop();
}

int CalendarClient_CalDAV::getYear() const
{
    return year_;
}

void CalendarClient_CalDAV::setYear(const int &year)
{
    year_ = year;
    emit yearChanged(year_);
}


int CalendarClient_CalDAV::getMonth() const
{
    return month_;
}

void CalendarClient_CalDAV::setMonth(const int &month)
{
   month_ = month;
   emit monthChanged(month_);
}

void CalendarClient_CalDAV::setUsername(const QString username)
{
  m_Username = username;
  emit usernameChanged(m_Username);
}

QString CalendarClient_CalDAV::getUsername(void) const
{
  return m_Username;
}

void CalendarClient_CalDAV::setPassword(const QString password)
{
  m_Password = password;
  emit passwordChanged(m_Password);
}

QString CalendarClient_CalDAV::getPassword(void) const
{
  return m_Password;
}

/*void CalendarClient_CalDAV::startSynchronization(void)
{
  qDebug() << m_DisplayName << ": " << "!!!forcing synchronization!!!";
  emit forceSynchronization();
}*/


void CalendarClient_CalDAV::stopSynchronization(void)
{
}

/*void CalendarClient_CalDAV::recover(void)
{
  qDebug() << m_DisplayName << ": " << "trying to recover from EEROR state";
  m_bRecoveredFromError = true;
  emit recoverSignal();
}*/

/*void CalendarClient_CalDAV::setYear(const int& year)
{
  if (E_STATE_IDLE == m_State)
  {
    if (m_Year != year)
    {
      qDebug() << m_DisplayName << ": " << "Year changed from" << m_Year << "to" << year;
      m_Year = year;
      emit yearChanged(m_Year);
      m_YearToBeRequested = year;
      startSynchronization();
    }
  }
  else
  {
    qDebug() << m_DisplayName << ": " << "requested Year changed from" << m_YearToBeRequested << "to" << year;
    m_YearToBeRequested = year;
  }
}

void CalendarClient_CalDAV::setMonth(const int& month)
{
  if (E_STATE_IDLE == m_State)
  {
    if (m_Month != month)
    {
      qDebug() << m_DisplayName << ": " << "Month changed from" << m_Month << "to" << month;
      m_Month = month;
      emit monthChanged(m_Month);
      m_MonthToBeRequested = month;
      startSynchronization();
    }
  }
  else
  {
    qDebug() << m_DisplayName << ": " << "requested Month changed from" << m_MonthToBeRequested << "to" << month;
    m_MonthToBeRequested = month;
  }
}*/

QNetworkReply* CalendarClient_CalDAV::getCTag(QOAuth2AuthorizationCodeFlow& google)
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, 0);
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setRawHeader("Depth", "0");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    root.setAttribute("xmlns:cs", REQUEST_URL);
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:displayname"));
    tagProp.appendChild(xml.createElement("cs:getctag"));
    root.appendChild(tagProp);

    auto reply = google.networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());

    /*connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });*/

    return reply;
}

void CalendarClient_CalDAV::getAllEvents(QOAuth2AuthorizationCodeFlow& google)
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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
    root.appendChild(tagFilter);

    auto reply = google.networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });
}

void CalendarClient_CalDAV::getDateRangeEvents(QOAuth2AuthorizationCodeFlow& google, QDateTime start, QDateTime end)
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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

    auto reply = google.networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });
}

void CalendarClient_CalDAV::requestSyncToken(QOAuth2AuthorizationCodeFlow& google)
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, 0);
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setRawHeader("Depth", "0");

    QDomDocument xml;
    QDomElement root = xml.createElement("d:propfind");
    root.setAttribute("xmlns:d", "DAV:");
    root.setAttribute("xmlns:cs", REQUEST_URL);
    xml.appendChild(root);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:displayname"));
    tagProp.appendChild(xml.createElement("cs:getctag"));
    tagProp.appendChild(xml.createElement("d:sync-token"));
    root.appendChild(tagProp);

    auto reply = google.networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());

    /*connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });*/
}

void CalendarClient_CalDAV::receiveChanges(QOAuth2AuthorizationCodeFlow& google)
{
    CalendarClient_CalDAV::requestSyncToken(google);

    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, 0);
    cal_part.setRawHeader("Prefer", "return-minimal");
    cal_part.setRawHeader("Depth", "0");

    QDomDocument xml;
    QDomElement root = xml.createElement("xml");
    root.setAttribute("version", "1.0");
    root.setAttribute("encoding", "utf-8");
    xml.appendChild(root);
    QDomElement tagCollection = xml.createElement("d:sync-collection");
    tagCollection.setAttribute("xmlns:d", "DAV:");
    xml.appendChild(tagCollection);
    QDomElement tagToken = xml.createElement("d:sync-token");
    tagToken.appendChild(xml.createTextNode(google.token().toUtf8()));
    tagCollection.appendChild(tagToken);
    QDomElement tagLevel = xml.createElement("d:sync-level");
    tagLevel.appendChild(xml.createTextNode("1"));
    tagCollection.appendChild(tagLevel);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:getetag"));
    tagCollection.appendChild(tagProp);

    auto reply = google.networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });
}

void CalendarClient_CalDAV::saveEvent(QOAuth2AuthorizationCodeFlow& google,
                                      CalendarEvent event)
{
  qDebug() << "saving event" << event.getUID();

  if (event.getUID().isEmpty())
  {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string = ("BEGIN:VCALENDAR\r\n" + event.ToICalendarObject() + "END:VCALENDAR\r\n").toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
  cal_part.setUrl(QUrl(QString(REQUEST_URL)+"/3nopsjhsq7dtugtjspkd1tlv91@google.com.ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, request_string.size());
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "0");

  //QSslConfiguration conf = request.sslConfiguration();
  //conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  //request.setSslConfiguration(conf);

  //Bisogna ottenere il .ics corretto (/home/lisa/calendars/events/qwue23489.ics)
  auto reply = google.networkAccessManager()->put(cal_part, request_string);

  qDebug() << "Put request sent";

  connect(reply, &QNetworkReply::finished, [reply]() {
    qDebug() << reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute);
  });
}


void CalendarClient_CalDAV::deleteEvent(QOAuth2AuthorizationCodeFlow& google, QString href)
{
  if (href.isEmpty())
  {
    return;
  }

  qDebug() << "deleting event with HREF" << href;

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Bearer "+google.token()).toUtf8());
  cal_part.setUrl(QUrl(QString(REQUEST_URL)+"/3nopsjhsq7dtugtjspkd1tlv91@google.com.ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, 0);
  cal_part.setRawHeader("Prefer", "return-minimal");
  cal_part.setRawHeader("Depth", "0");

  auto reply = google.networkAccessManager()->deleteResource(cal_part);

  connect(reply, &QNetworkReply::finished, []() {

  });
}

/*
void CalendarClient_CalDAV::handleUploadHTTPError(void)
{
  upload_request_timeout_timer_.stop();
  if (NULL != upload_reply_)
  {
    qDebug() << "HTTP upload error:" << upload_reply_->errorString();
    //emit error(m_pUploadReply->errorString());
  }
  else
  {
    qDebug() << "ERROR: Invalid reply pointer when handling HTTP error.";
    //emit error("Invalid reply pointer when handling HTTP error.");
  }

}

void CalendarClient_CalDAV::handleUploadFinished(void)
{
  upload_request_timeout_timer_.stop();

  qDebug() << "HTTP upload finished";

  if (NULL != upload_reply_)
  {
    qDebug() << "received:\r\n" << upload_reply_->readAll();
    //emit forceSynchronization();
  }
}

*/
/***** End of: pubslots Public slots ************************************//*@}*/

/******************************************************************************/
/* Protected slots                                                            */
/*********************//*!@addtogroup protslots Protected slots         *//*@{*/
/***** End of: protslots Protected slots ********************************//*@}*/

/******************************************************************************/
/* Private slots                                                              */
/*************************//*!@addtogroup privslots Private slots       *//*@{*/
/***** End of:privslots Private slots ***********************************//*@}*/

/**** Last line of source code                                             ****/
