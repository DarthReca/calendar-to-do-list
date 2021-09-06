#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QDateTime>
#include <QObject>
#include <QString>

class CalendarEvent {
 public:
  CalendarEvent();

  virtual CalendarEvent& fromICalendar(QTextStream& icalendar);
  virtual QString toICalendar();

  QList<QDateTime> recurrencesInRange(QDateTime from, QDateTime to);

  ///////// STATIC UTILS //////

  static QDateTime dateTimeFromString(const QString& date_time_string);
  static int weekDayFromString(const QString& weekday_string);
  static QString stringFromWeekDay(int weekday);

  //////// OPERATORS //////////

  bool operator<(const CalendarEvent& other) const {
    return (this->start_date_time_ < other.start_date_time_);
  };
  bool operator==(const CalendarEvent& other) const {
    return (this->uid_ == other.uid_);
  };

  //////// GETTERS SETTERS /////////

  const QString& eTag() const;
  void setETag(const QString& new_eTag);

  bool all_day() const;
  void setAllDay(bool newAll_day);

  QString getColor() const;
  void setColor(const QString& color);

  QString calendarName() const;
  void setCalendarName(const QString& calendarName);

  QString summary() const;
  void setSummary(const QString& name);

  QString location() const;
  void setLocation(const QString& location);

  QString description() const;
  void setDescription(const QString& description);

  QDateTime startDateTime() const;
  void setStartDateTime(const QDateTime& startDateTime);

  QDateTime endDateTime() const;
  void setEndDateTime(const QDateTime& endDateTime);

  QString RRULE() const;
  void setRRULE(const QString& rRule);

  QString exdates() const;
  void setExdates(const QString& exdates);

  QString categories() const;
  void setCategories(const QString& categories);

  QString uid() const;
  void setUid(const QString& uid);

  QString href() const;
  void setHref(const QString& href);

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
  QString uid_;
  QString href_;

  QString eTag_;
};

#endif  // CALENDAREVENT_H
