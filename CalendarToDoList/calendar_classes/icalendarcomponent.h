#ifndef ICALENDARCOMPONENT_H
#define ICALENDARCOMPONENT_H

#include <QDateTime>
#include <QHash>
#include <QTextStream>

class ICalendarComponent {
 public:
  ICalendarComponent();

  static ICalendarComponent fromICalendar(QTextStream& icalendar,
                                          const QString& type);
  QString toICalendar();

  static QDateTime dateTimeFromString(const QString& date_time_string);
  static int weekDayFromString(const QString& weekday_string);
  static QString stringFromWeekDay(int weekday);

  std::optional<QString> getProperty(const QString& name);
  void setProperty(const QString& key, const QString& value);
  void removeProperty(const QString& name);

  std::optional<QDateTime> getStartDateTime();
  void setStartDateTime(const QDateTime& start);

  std::optional<QDateTime> getEndDateTime();
  void setEndDateTime(const QDateTime& end);

  QString getUID();

  const QString& href() { return href_; };
  void setHref(const QString& href) { href_ = href; };

  const QString& eTag() { return eTag_; };
  void setEtag(const QString& eTag) { eTag_ = eTag; };

  const QString& type() { return type_; };
  void setType(const QString& type) { type_ = type; };

  const bool& allDay() { return all_day_; };
  void setAllDay(const bool& all_day);

 private:
  QString href_;
  QString eTag_;

  QString type_;

  bool all_day_;

  QHash<QString, QString> properties_;
};

#endif  // ICALENDARCOMPONENT_H
