/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
* File identification:      $Id: CalendarClient.h 16 2017-01-26 17:11:19Z cypax $
* Revision of last commit:  $Rev: 16 $
* Author of last commit:    $Author: cypax $
* Date of last commit:      $Date: 2017-01-26 18:11:19 +0100 (Do, 26 Jan 2017) $
 *******************************************************************************
 *
 * @file          CalendarClient.h
 * @author        Cypax (cypax.net)
 *
 * @brief         Common parent class for all calendars. Contians iCalendar parsing.
 */
/************************************************************************//*@}*/

#ifndef CALENDARCLIENT_H
#define CALENDARCLIENT_H

/******************************************************************************/
/* Library includes                                                           */
/******************************************************************************/
#include <QObject>
#include <QTimer>
#include <QList>
#include <QDate>
#include <QUrl>
#include <QTextStream>
#include <QtNetwork>

/******************************************************************************/
/* Own includes                                                               */
/******************************************************************************/
#include "CalendarEvent.h"

/******************************************************************************/
/* Namespace                                                                 */
/******************************************************************************/

/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

// defines wether to use the color as specified in the calendar source (1) or
// to use a user defined color (0)
#define CALENDAR_OVERWRITE_COLOR 0

/***** End of: define Constants *****************************************//*@}*/

/******************************************************************************/
/* Class                                                                      */
/******************************************//*!@addtogroup class Classes*//*@{*/
/**
* @brief Abstract base class for all calendar classes which obtain CalendarEvent objects from local or remote sources.
**/
class CalendarClient : public QObject
{
  Q_OBJECT

  Q_ENUMS(E_CalendarState)
  Q_ENUMS(E_CalendarType)

/******************************************************************************/
/* Types (typedef)                                                            */
/******************************************//*!@addtogroup typedef Types*//*@{*/
public:

  typedef enum {
    E_STATE_IDLE,  // calendar is waiting for next synchronization
    E_STATE_BUSY,  // calendar is currently synchronizing
    E_STATE_ERROR  // calendar has encountered an error condition
  } E_CalendarState;

  typedef enum {
    E_CALENDAR_ICS,     // event source is a local or remote iCalendar file
    E_CALENDAR_CALDAV   // event source is a CalDAV server
  } E_CalendarType;

/***** End of: typedef Types ********************************************//*@}*/

/******************************************************************************/
/* Properties (Q_PROPERTY)                                                    */
/**********************************//*!@addtogroup Q_PROPERTY Properties*//*@{*/

  // Assinged color of this CalendarClient for graphical illustration in GUIs
  Q_PROPERTY(QString color READ getColor WRITE setColor NOTIFY colorChanged)

  // Synchronization state of the CalendarClient
  Q_PROPERTY(E_CalendarState syncState READ getSyncState NOTIFY syncStateChanged)

  // Type of calendar; i.e. what the source of calendar events is
  Q_PROPERTY(E_CalendarType calendarType READ getCalendarType CONSTANT)

  // URL of calendar events source (iCalendar file path, CalDAV server URL, ...)
  Q_PROPERTY(QUrl hostURL READ getHostURL WRITE setHostURL NOTIFY hostURLChanged)

  // Displayed name of the CalendarClient instance; might be overwritten by received calendar information
  Q_PROPERTY(QString displayName READ getDisplayName WRITE setDisplayName NOTIFY displayNameChanged)

  // Request timeout in ms for accessing remote calendar sources (default: 2000ms)
  Q_PROPERTY(int    requestTimeoutMS
             READ   getRequestTimeoutMS
             WRITE  setRequestTimeoutMS
             NOTIFY requestTimeoutMSChanged)

/***** End of: Q_PROPERTY Properties ************************************//*@}*/

/******************************************************************************/
/* Constructors                                                               */
/******************************************************************************/
public:
  CalendarClient(QObject* parent = NULL);

/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
public:
  ~CalendarClient();

/******************************************************************************/
/* Public function prototypes                                                 */
/******************************************************************************/
public:

  /**
   * @brief Returns a list of events which occur on the questioned date.
   */
  Q_INVOKABLE QList<QObject*> eventsForDate(const QDate& date);

  /**
   * @brief Returns the complete list of events this CalendarClient manages.
   */
  Q_INVOKABLE QList<QObject*> allEvents(void);

/******************************************************************************/
/* Protected function prototypes                                                */
/******************************************************************************/
protected:

  /**
   * @brief Helper function to encode network authorization requests.
   */
  QString encodeBase64(QString string);

  /**
   * @brief Helper function to decode received strings.
   */
  QString ascii2utf8(QString str);

  /**
   * @brief Parses the CALENDAR section of an iCalendar file.
   */
  void parseCALENDAR(QString href);

  /**
   * @brief Parses the VEVENT section of an iCalendar file.
   */
  void parseVEVENT(QString href);

  /**
   * @brief Adds a single occurrence event to the m_EventList if the event occurs within a given start and end QDateTime.
   * @note Single occurrence event = event without RRULE value.
   * @note Events with a RRULE value are handled within parseVEVENT()
   */
  QList<QObject*> handleSingleEvent(CalendarEvent& evt, const QDateTime& startOfQuestionedDate, const QDateTime& endOfQuestionedDate);

  /**
   * @brief Returns true, if a questioned date is excluded by the event EXDATE value.
   */
  bool isDateExcluded(const QString strExdates, const QDate& questionedDate) const;

/******************************************************************************/
/* Private function prototypes                                                */
/******************************************************************************/
private:

/******************************************************************************/
/* Signals                                                                    */
/******************************************************************************/
signals:

  void colorChanged(QString color);
  void syncStateChanged(E_CalendarState syncState); // emitted when the calendar has entered a new synchronization state
  void calendarTypeChanged(E_CalendarType calendarType);
  void hostURLChanged(QString hostURL);
  void displayNameChanged(QString hostURL);

  // emitted, when the events in m_EventList have changed
  void eventsUpdated(void);

  // emitted, when an error situation occurred
  void error(QString errorMsg);

  // emitted when the error state shall be recovered
  void recoverSignal(void);

  void requestTimeoutMSChanged(int);

  void forceSynchronization(void); // emitted to force a synchronization with the calendar source (file or server)

/******************************************************************************/
/* Public slots                                                               */
/******************************************************************************/
public slots:

  /**
   * @brief Slot to start synchronization with event source.
   */
  virtual void startSynchronization(void) = 0;

  /**
   * @brief Slot to stop/abort synchronization with event source.
   */
  virtual void stopSynchronization(void) = 0;

  /**
   * @brief Slot to recover from error state.
   */
  virtual void recover(void) = 0;


  QString getColor(void) const;
  void setColor(const QString& color);

  E_CalendarState getSyncState(void);
  E_CalendarType getCalendarType(void);

  virtual bool setHostURL(const QUrl hostURL);
  QString getHostURL(void) const;

  QString getDisplayName(void) const;
  virtual void setDisplayName(QString name);

  int getRequestTimeoutMS(void) const;
  void setRequestTimeoutMS(const int requestTimeoutMS);

/******************************************************************************/
/* Protected slots                                                            */
/******************************************************************************/
protected slots:

/******************************************************************************/
/* Private slots                                                               */
/******************************************************************************/
private slots:

/******************************************************************************/
/* Protected attributes                                                         */
/******************************************************************************/
protected:
  QString               m_Color;
  E_CalendarState       m_State;
  E_CalendarType        m_CalendarType;
  QUrl                  m_HostURL;
  QString               m_DisplayName;

  // Timer to automatically re-synchronize with event source
  QTimer                m_SynchronizationTimer;

  // Test stream to handle iCalendar file content
  QTextStream*          m_DataStream;

  // List of managed CalendarEvent events
  QList<CalendarEvent>  m_EventList;

  QNetworkAccessManager m_NetworkManager;
  QNetworkReply* m_pReply;

  QTimer m_RequestTimeoutTimer;
  int m_RequestTimeoutMS;

/******************************************************************************/
/* Private attributes                                                         */
/******************************************************************************/
private:

};

/***** End of: Classes **************************************************//*@}*/

#endif // CALENDARCLIENT_H
/**** Last line of source code                                             ****/





