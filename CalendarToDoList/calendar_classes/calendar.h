#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>
#include "calendarevent.h"

class Calendar : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayName READ getDisplayName WRITE setDisplayName NOTIFY displayNameChanged)
public:
    Calendar(QObject *parent = nullptr);
    Calendar(const QString& href, QTextStream& ical_object, QObject *parent = nullptr);

    QString ToICalendarObject();

    ~Calendar();

    QVector<CalendarEvent> &events();
    void setEvents(const QVector<CalendarEvent> &newEvents);
    void addEvent(CalendarEvent newEvent);
signals:
  void displayNameChanged(QString display_name);
public slots:
  QString getDisplayName(void) const;
  void setDisplayName(QString name);
private:
  QString display_name_;
  QVector<CalendarEvent> events_;
};

#endif // CALENDAR_H
