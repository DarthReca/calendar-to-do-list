#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QDateTime>
#include <QObject>
#include <QString>

class CalendarEvent : public QObject {
  Q_OBJECT

 public:
  CalendarEvent(QObject* parent = nullptr);
  CalendarEvent(QTextStream& ical_object, QObject* parent);
  CalendarEvent(const CalendarEvent& other);

  static CalendarEvent FromiCalendar(const QString& icalendar);
  virtual QString toiCalendar();

  QList<QDateTime> RecurrencesInRange(QDateTime from, QDateTime to);

  ///////// STATIC UTILS //////

  static QDateTime DateTimeFromString(const QString& date_time_string);
  static int WeekDayFromString(const QString& weekday_string);
  static QString StringFromWeekDay(int weekday);

  //////// OPERATORS //////////

  CalendarEvent& operator=(const CalendarEvent& other);

  bool operator<(const CalendarEvent& other) const {
    return (this->start_date_time_ < other.start_date_time_);
  };
  bool operator==(const CalendarEvent& other) const {
    return (this->UID_ == other.UID_);
  };

  void copyFrom(const CalendarEvent& other);

  //////// GETTERS SETTERS /////////

  const QString& eTag() const;
  void setETag(const QString& newETag);

  bool all_day() const;
  void setAll_day(bool newAll_day);

 signals:
  void colorChanged(QString& color);
  void calendarNameChanged(const QString& calendarName);
  void nameChanged(const QString& name);
  void locationChanged(const QString& location);
  void descriptionChanged(const QString& description);
  void startDateTimeChanged(const QDateTime& startDateTime);
  void endDateTimeChanged(const QDateTime& endDateTime);
  void rruleChanged(const QString& rrule);
  void exdatesChanged(const QString& exdates);
  void categoriesChanged(const QString& categories);
  void uidChanged(const QString& uid);
  void hrefChanged(const QString& href);

 public slots:
  QString getColor(void) const;
  void setColor(const QString& color);

  QString calendarName() const;
  void setCalendarName(const QString& calendarName);

  QString summary() const;
  void setSummary(const QString& name);

  QString location() const;
  void setLocation(const QString& location);

  QString description() const;
  void setDescription(const QString& description);

  QDateTime getStartDateTime() const;
  void setStartDateTime(const QDateTime& startDateTime);

  QDateTime getEndDateTime(void) const;
  void setEndDateTime(const QDateTime& endDateTime);

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

 protected:
  /**
   * @brief Transform rrule string to a map
   */
  QHash<QString, QString> parseRRule();

  QString color_;
  QString calendar_name_;
  QString summary_;
  QString location_;
  QString description_;

  QDateTime start_date_time_;
  QDateTime end_date_time_;
  bool all_day_;

  QString RRULE_;
  QString exdates_;
  QString categories_;
  QString UID_;
  QString HREF_;

  QString eTag_;
};

#endif  // CALENDAREVENT_H
