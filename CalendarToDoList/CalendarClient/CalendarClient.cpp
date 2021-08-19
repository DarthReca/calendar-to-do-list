#include <QByteArray>
#include <QString>
#include <QMap>
#include <QDomDocument>

#include "calendarclient.h"
#include "calendar_classes/calendar.h"

#define REQUEST_URL "https://apidata.googleusercontent.com/caldav/v2/k8tsgo0usjdlipul5pb2vel68o@group.calendar.google.com/events"

CalendarClient::CalendarClient(GoogleAuth& auth, QObject* parent)
{
  auth_ = &auth;
}

CalendarClient::~CalendarClient()
{
    delete auth_;
}

QNetworkReply* CalendarClient::obtainCTag()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+ auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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

    return auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());
}

QNetworkReply* CalendarClient::getAllEvents()
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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

    return auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());
}

QNetworkReply* CalendarClient::lookForChanges(){
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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

    return auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());
}

QNetworkReply* CalendarClient::getChangedEvents(){
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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
    for(QString Item : changedItems_){
        tagHref = xml.createElement("d:href");
        tagHref.appendChild(xml.createTextNode(Item));
        root.appendChild(tagHref);
    }

    return auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());
}

void CalendarClient::getDateRangeEvents(QDateTime start, QDateTime end)
{
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
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

    auto reply = auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });
}

QNetworkReply* CalendarClient::requestSyncToken()
{
    /*
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl(REQUEST_URL));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
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

    auto reply = auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PROPFIND"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->readAll();
    });
    */
    return nullptr;
}

void CalendarClient::receiveChanges(QString syncToken)
{
    /*
    QNetworkRequest cal_part;
    cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
    cal_part.setUrl(QUrl("https://apidata.googleusercontent.com/caldav/v2/k8tsgo0usjdlipul5pb2vel68o@group.calendar.auth_->google->com/events/sync"));
    cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/xml; charset=utf-8");
    cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");

    QDomDocument xml;
    QDomElement root = xml.createElement("?xml");
    root.setAttribute("version", "1.0");
    root.setAttribute("encoding", "utf-8");
    xml.appendChild(root);
    QDomElement tagCollection = xml.createElement("d:sync-collection");
    tagCollection.setAttribute("xmlns:d", "DAV:");
    xml.appendChild(tagCollection);
    QDomElement tagToken = xml.createElement("d:sync-token");
    tagToken.appendChild(xml.createTextNode("https://apidata.googleusercontent.com" + syncToken));
    tagCollection.appendChild(tagToken);
    QDomElement tagLevel = xml.createElement("d:sync-level");
    tagLevel.appendChild(xml.createTextNode("1"));
    tagCollection.appendChild(tagLevel);
    QDomElement tagProp = xml.createElement("d:prop");
    tagProp.appendChild(xml.createElement("d:getetag"));
    tagCollection.appendChild(tagProp);

    qDebug() << xml.toString();

    auto reply = auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("REPORT"), xml.toByteArray());

    connect(reply, &QNetworkReply::finished, [reply]() {
      qDebug() << reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute);
    });
    */
}

void CalendarClient::updateEvent(CalendarEvent event, QDomElement eTag)
{
  qDebug() << "updating an existing event: " << event.getUID();

  if (event.getUID().isEmpty())
  {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string = ("BEGIN:VCALENDAR\r\n" + event.ToICalendarObject() + "END:VCALENDAR\r\n").toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(REQUEST_URL)+"/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag.text());
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");

  //QSslConfiguration conf = request.sslConfiguration();
  //conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  //request.setSslConfiguration(conf);

  //Bisogna ottenere il .ics corretto (/home/lisa/calendars/events/qwue23489.ics)
  auto reply = auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PUT"), request_string);

  qDebug() << "Put request sent\n";
  qDebug() << reply->readAll();

  connect(reply, &QNetworkReply::finished, [reply]() {
    qDebug() << reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute);
  });
}

void CalendarClient::saveEvent(CalendarEvent& event)
{
  qDebug() << "saving new event:\n\n" << event.ToICalendarObject();

  if (event.getUID().isEmpty())
  {
    event.setUID(QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + event.getStartDateTime().toString("yyyyMMddHHMM"));
  }

  QByteArray request_string = ("BEGIN:VCALENDAR\r\n" + event.ToICalendarObject() + "END:VCALENDAR\r\n").toUtf8();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(REQUEST_URL)+"/" + event.getUID() + ".ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "text/calendar; charset=utf-8");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");

  //QSslConfiguration conf = request.sslConfiguration();
  //conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  //request.setSslConfiguration(conf);

  //Bisogna ottenere il .ics corretto (/home/lisa/calendars/events/qwue23489.ics)
  auto reply = auth_->google->networkAccessManager()->sendCustomRequest(cal_part, QByteArray("PUT"), request_string);

  qDebug() << "Put request sent\n";

  connect(reply, &QNetworkReply::finished, [this, reply]() {
      qDebug() << reply->rawHeaderPairs();
      qDebug() << "\n";

      //checkForChanges();
  });
}


void CalendarClient::deleteEvent(QDomElement eTag)
{
  if (eTag.text().isEmpty())
  {
    return;
  }

  qDebug() << "deleting event with eTag" << eTag.text();

  QNetworkRequest cal_part;
  cal_part.setRawHeader("Authorization", ("Bearer "+auth_->google->token()).toUtf8());
  cal_part.setUrl(QUrl(QString(REQUEST_URL)+"/3nopsjhsq7dtugtjspkd1tlv91@google.com.ics"));
  cal_part.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "CalendarClient_CalDAV");
  cal_part.setHeader(QNetworkRequest::KnownHeaders::IfMatchHeader, eTag.text());

  auto reply = auth_->google->networkAccessManager()->deleteResource(cal_part);

  connect(reply, &QNetworkReply::finished, []() {

  });
}

void CalendarClient::checkForChanges()
{

}

/*void CalendarClient::checkForChanges()
{
    auto reply = this->obtainCTag();
        connect(reply, &QNetworkReply::finished, [this, reply]() mutable {
          QDomDocument res;
          res.setContent(reply->readAll());
          auto newCTag = res.elementsByTagName("cs:getctag").at(0).toElement();

          if(newCTag.text() == this->getCTag().text()){
              qDebug() << "Calendar already up to date";
          }
          else{
              this->setCTag(newCTag);
              reply = this->lookForChanges();
              connect(reply, &QNetworkReply::finished, [this, reply]() {

                  //creo una mappa con gli href e gli etag nuovi
                  QDomDocument res;
                  res.setContent(reply->readAll());
                  auto hrefs_list = res.elementsByTagName("D:href");
                  auto eTags = res.elementsByTagName("D:getetag");
                  QMap<QString, QDomElement> mapTmp;

                  qDebug() << "Mappa dopo:\n\n";
                  for(int i=0; i<eTags.size(); i++){
                      mapTmp.insert(hrefs_list.at(i).toElement().text(), eTags.at(i).toElement());
                      qDebug() << hrefs_list.at(i).toElement().text() + " - " + eTags.at(i).toElement().text() + "\n\n";

                  }
                  qDebug() << "\n\n";

                  //confronto la nuova mappa con quella esistente
                  //e aggiorno la lista di eTag nel client
                  QMap<QString, QDomElement> oldMap = this->getETags();
                  QMap<QString, QDomElement>::iterator i;
                  for(i = oldMap.begin(); i != oldMap.end(); ++i){
                      if(mapTmp.contains(i.key())){
                          if(mapTmp[i.value().text()]!=oldMap[i.value().text()]){
                              qDebug() << "Item with href " + i.key() + "has a new etag: " + i.value().text() + "\n\n";
                              this->addChangedItem(i.key());
                              this->deleteETag(i.key());
                              this->addETag(i.key(), i.value());
                          }
                      }
                      else{
                          qDebug() << "Item with eTag " + i.value().text() + "has been deleted\n\n";
                          this->addDeletedItem(i.key());
                          this->deleteETag(i.key());
                      }
                  }
                  for(i = mapTmp.begin(); i != mapTmp.end(); ++i){
                      if(!oldMap.contains(i.key())){
                          qDebug() << "There is a new Item with eTag: " + i.value().text() + "\n\n";
                          this->addChangedItem(i.key());
                          this->addETag(i.key(), i.value());
                      }
                  }

                  auto reply = this->getChangedEvents();
                  connect(reply, &QNetworkReply::finished, [this, reply](){
                      QDomDocument res;
                      res.setContent(reply->readAll());
                      auto events = res.elementsByTagName("caldav:calendar-data");
                      auto href_list = res.elementsByTagName("D:href");
                      for(int i=0; i<events.size(); i++){
                          qDebug() << events.at(i).toElement().text() + "\n\n";

                          //salvo l'evento nella lista di eventi del calendario
                          QString el = events.at(i).toElement().text();
                          QTextStream stream(&el);
                          QPointer<Calendar> tmp = new Calendar("href_list.at(i).toElement().text()", stream);
                          if(calendar_.isNull())
                              calendar_ = tmp;
                          else
                              calendar_->events().append(tmp->events());
                      }
                      if(ui->calendarTable->columnCount() == 7)
                          on_actionSettimanale_triggered();
                      else
                          on_actionGiorno_triggered();
                  });

              });
          }
    });
}*/
