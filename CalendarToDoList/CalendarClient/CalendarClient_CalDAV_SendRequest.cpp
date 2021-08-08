/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
* File identification:      $Id: CalendarClient_CalDAV_SendRequest.cpp 18 2017-01-26 17:33:06Z cypax $
* Revision of last commit:  $Rev: 18 $
* Author of last commit:    $Author: cypax $
* Date of last commit:      $Date: 2017-01-26 18:33:06 +0100 (Do, 26 Jan 2017) $
 *******************************************************************************
 *
 * @file          CalendarClient_CalDAV_StateMachine.cpp
 * @author        Cypax (cypax.net)
 *
 * @brief         Methods to send HTTP(S) requests to CalDAV server.
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* Library includes                                                           */
/******************************************************************************/
#include <QDebug>
#include <QXmlSimpleReader>
#include <QXmlContentHandler>
#include <QXmlQuery>
#include <QXmlStreamNamespaceDeclaration>

/******************************************************************************/
/* Own includes                                                               */
/******************************************************************************/
#include "CalendarClient_CalDAV.h"
#include "DateUtils.h"

/******************************************************************************/
/* Namespace                                                                 */
/******************************************************************************/

/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

#define DEBUG_CALENDARCLIENT_CALDAV_SENDREQUEST 0
#if DEBUG_CALENDARCLIENT_CALDAV_SENDREQUEST
#define QDEBUG qDebug()
#else
#define QDEBUG if (0) qDebug()
#endif

/***** End of: define Constants *****************************************//*@}*/

/******************************************************************************/
/* Constructors                                                               */
/**********************************//*!@addtogroup constrc Constructors *//*@{*/
/***** End of: constrc Constructors *************************************//*@}*/

/******************************************************************************/
/* Deconstructor                                                              */
/******************************//*!@addtogroup deconstrc Deconstructors *//*@{*/
/***** End of: deconstrc Constructors ***********************************//*@}*/

/******************************************************************************/
/* Public functions                                                           */
/*****************************//*!@addtogroup pubfunct Public functions *//*@{*/
/***** End of: pubfunct Public functions ********************************//*@}*/

/******************************************************************************/
/* Protected functions                                                        */
/*************************//*!@addtogroup protfunct Protected functions *//*@{*/


void CalendarClient_CalDAV::sendRequestSyncToken(void)
{
  if (NULL != m_pReply)
  {
    QDEBUG << m_DisplayName << ": " << "cleaning up m_pReply";
    m_pReply->abort();
    m_pReply->disconnect();
  }

  QString authorization = "Basic ";
  authorization.append(encodeBase64(m_Username + ":" + m_Password));

  QBuffer* buffer = new QBuffer();

  buffer->open(QIODevice::ReadWrite);

  QString requestString = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\">\r\n"
                          "  <d:prop>\r\n"
                          "    <d:displayname />\r\n"
                          "    <cs:getctag />\r\n"
                          "    <d:sync-token />"
                          "  </d:prop>\r\n"
                          "</d:propfind>";

  int buffersize = buffer->write(requestString.toUtf8());
  buffer->seek(0);
  buffer->size();

  QByteArray contentlength;
  contentlength.append(QString::number(buffersize));

  QNetworkRequest request;
  request.setUrl(m_HostURL);
  request.setRawHeader("User-Agent", "CalendarClient_CalDAV");
  request.setRawHeader("Authorization", authorization.toUtf8());
  request.setRawHeader("Depth", "0");
  request.setRawHeader("Prefer", "return-minimal");
  request.setRawHeader("Content-Type", "text/xml; charset=utf-8");
  request.setRawHeader("Content-Length", contentlength);

  QSslConfiguration conf = request.sslConfiguration();
  conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  request.setSslConfiguration(conf);

  m_pReply = m_NetworkManager.sendCustomRequest(request, QByteArray("PROPFIND"), buffer);

  if (NULL != m_pReply)
  {
    connect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleHTTPError()));

    connect(m_pReply, SIGNAL(finished()),
            this, SLOT(handleRequestSyncTokenFinished()));

    m_RequestTimeoutTimer.start(m_RequestTimeoutMS);
  }
  else
  {
    QDEBUG << m_DisplayName << ": " << "ERROR: Invalid reply pointer when requesting sync token.";
    emit error("Invalid reply pointer when requesting sync token.");
  }
}



void CalendarClient_CalDAV::sendRequestChanges(void)
{
  QString authorization = "Basic ";
  authorization.append(encodeBase64(m_Username + ":" + m_Password));

  QBuffer* buffer = new QBuffer();

  buffer->open(QIODevice::ReadWrite);

  QString monthString = QString::number(m_Month);
  if (monthString.length() < 2) monthString.prepend("0");

  QString lastDayString = QString::number(DateUtils::lastDayOfMonth(m_Year, m_Month));
  if (lastDayString.length() < 2) lastDayString.prepend("0");


  QString requestString = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
                          "<C:calendar-query xmlns:D=\"DAV:\" xmlns:C=\"urn:ietf:params:xml:ns:caldav\">\r\n"
                          " <D:prop>\r\n"
                          "   <D:getetag/>\r\n"
                          "   <C:calendar-data>\r\n"
                          "     <C:comp name=\"VCALENDAR\">\r\n"
                          "       <C:prop name=\"VERSION\"/>\r\n"
                          "       <C:comp name=\"VEVENT\">\r\n"
                          "         <C:prop name=\"SUMMARY\"/>\r\n"
                          "         <C:prop name=\"LOCATION\"/>\r\n"
                          "         <C:prop name=\"DESCRIPTION\"/>\r\n"
                          "         <C:prop name=\"UID\"/>\r\n"
                          "         <C:prop name=\"DTSTART\"/>\r\n"
                          "         <C:prop name=\"DTEND\"/>\r\n"
                          "       </C:comp>\r\n"
                          "     </C:comp>\r\n"
                          "   </C:calendar-data>\r\n"
                          " </D:prop>\r\n"
                          " <C:filter>\r\n"
                          "   <C:comp-filter name=\"VCALENDAR\">\r\n"
                          "     <C:comp-filter name=\"VEVENT\">\r\n"
                          "       <C:time-range start=\"" + QString::number(m_Year) + monthString + "01T000000Z\" end=\"" + QString::number(m_Year) + monthString + lastDayString + "T235959Z\"/>\r\n"
                          "     </C:comp-filter>\r\n"
                          "   </C:comp-filter>\r\n"
                          " </C:filter>\r\n"
                          "</C:calendar-query>\r\n";

  //QDEBUG << m_DisplayName << ": " << "requesting:";
  //QDEBUG << m_DisplayName << ": " << requestString;
  //QDEBUG << m_DisplayName << ": " << "<C:time-range start=\"" + QString::number(m_Year) + monthString + "01T000000Z\" end=\"" + QString::number(m_Year) + monthString + lastDayString + "T235959Z\"/>";

  int buffersize = buffer->write(requestString.toUtf8());
  buffer->seek(0);
  buffer->size();

  QByteArray contentlength;
  contentlength.append(QString::number(buffersize));

  QNetworkRequest request;
  request.setUrl(m_HostURL);
  request.setRawHeader("User-Agent", "CalendarClient_CalDAV");
  request.setRawHeader("Authorization", authorization.toUtf8());
  request.setRawHeader("Depth", "1");
  request.setRawHeader("Content-Type", "application/xml; charset=utf-8");
  request.setRawHeader("Content-Length", contentlength);

  QSslConfiguration conf = request.sslConfiguration();
  conf.setPeerVerifyMode(QSslSocket::VerifyNone);
  request.setSslConfiguration(conf);

  m_pReply = m_NetworkManager.sendCustomRequest(request, QByteArray("REPORT"), buffer);

  if (NULL != m_pReply)
  {
    connect(m_pReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleHTTPError()));

    connect(m_pReply, SIGNAL(finished()),
            this, SLOT(handleRequestChangesFinished()));

    m_RequestTimeoutTimer.start(m_RequestTimeoutMS);
  }
  else
  {
    QDEBUG << m_DisplayName << ": " << "ERROR: Invalid reply pointer when requesting sync token.";
    emit error("Invalid reply pointer when requesting sync token.");
  }
}



/***** End of: protfunct Protected functions ****************************//*@}*/

/******************************************************************************/
/* Private functions                                                          */
/*************************//*!@addtogroup privfunct Private functions   *//*@{*/
/***** End of: privfunct Private functions ******************************//*@}*/

/******************************************************************************/
/* Public slots                                                               */
/*************************//*!@addtogroup pubslots Public slots         *//*@{*/
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
