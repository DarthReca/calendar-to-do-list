#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QDateTime>
#include <QObject>
#include <QString>

class CalendarEvent : public QObject
{
  Q_OBJECT

public:

  CalendarEvent(QObject* parent = nullptr);
  CalendarEvent(const QString& href, QTextStream& ical_object, QObject* parent);
  CalendarEvent(const CalendarEvent& other);

  QString ToICalendarObject();
  QList<QDateTime> RecurrencesInRange(QDateTime from, QDateTime to);

  CalendarEvent& operator=(const CalendarEvent& other);
  bool operator<(const CalendarEvent& other) const;

  void copyFrom(const CalendarEvent& other);

signals:
  void colorChanged(QString& color);
  void calendarNameChanged(const QString& calendarName);
  void nameChanged(const QString &name);
  void locationChanged(const QString& location);
  void descriptionChanged(const QString& description);
  void startDateTimeChanged(const QDateTime &startDateTime);
  void endDateTimeChanged(const QDateTime &endDateTime);
  void rruleChanged(const QString& rrule);
  void exdatesChanged(const QString& exdates);
  void categoriesChanged(const QString& categories);
  void uidChanged(const QString& uid);
  void hrefChanged(const QString& href);

public slots:
  QString getColor(void) const;
  void setColor(const QString& color);

  QString calendarName() const;
  void setCalendarName(const QString &calendarName);

  QString name() const;
  void setName(const QString &name);

  QString location() const;
  void setLocation(const QString& location);

  QString description() const;
  void setDescription(const QString& description);

  QDateTime getStartDateTime() const;
  void setStartDateTime(const QDateTime &startDateTime);

  QDateTime getEndDateTime(void) const;
  void setEndDateTime(const QDateTime &endDateTime);

  QString getRRULE() const;
  void setRRULE(const QString& rRule);

  QString getExdates() const;
  void setExdates(const QString& exdates);

  QString getCategories() const;
  void setCategories(const QString& categories);

  QString getUID(void) const;
  void setUID(const QString& uid);

  QString getHREF(void) const;
  void setHREF(const QString& href);


  // to edit an event and upload it back to the CalDAV server we need to know the
  // CalendarClient object where this event originates from
  QObject* getCalendarPointer(void) const;
  void setCalendarPointer(QObject* ptr);

private:
  QHash<QString, QString> parseRRule();

  QString color_;
  QString calendar_name_;
  QString name_;
  QString location_;
  QString description_;
  QDateTime start_date_time_;
  QDateTime end_date_time_;
  QString RRULE_;
  QString exdates_;
  QString categories_;
  QString UID_;
  QString HREF_;
  QObject* calendar_pointer_;

};

#endif // CALENDAREVENT_H
