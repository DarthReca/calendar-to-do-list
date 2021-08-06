#include "calendar.h"

Calendar::Calendar(QObject *parent) : QObject(parent)
{

}

void Calendar::LoadFromReply(QNetworkReply& reply)
{
    if(!reply.isFinished() || reply.size() == 0)
        return;

    Component current_component = VCALENDAR, prev_component = VCALENDAR;
    Event *new_event = NULL;
    Alarm new_alarm;
    QByteArray line;
    QByteArray next_line = reply.readLine();
    unsigned int no_uid = 0;

    while (reply.canReadLine()) {
        line = next_line;
        // lines can be wrapped after 75 octets so we may have to unwrap them
        for (;;) {
            next_line = reply.readLine();
            if (next_line[0] != '\t' && next_line[0] != ' ')
                break;
            line += next_line.sliced(1);
        }

        switch (current_component) {
            case VCALENDAR:
                if (line.indexOf("BEGIN:VEVENT") == 0) {
                    new_event = new Event;
                    current_component = VEVENT;
                }
                break;

            case VEVENT:
                if (line.indexOf("UID") == 0) {
                    new_event->uid_ = GetProperty(line);
                } else if (line.indexOf("SUMMARY") == 0) {
                    new_event->summary_ = GetProperty(line);
                } else if (line.indexOf("DTSTAMP") == 0) {
                    new_event->dt_stamp_ = QDateTime::fromString(GetProperty(line), Qt::DateFormat::ISODate);
                } else if (line.indexOf("DTSTART") == 0) {
                    new_event->dt_start_ = QDateTime::fromString(GetProperty(line), Qt::DateFormat::ISODate);
                } else if (line.indexOf("DTEND") == 0) {
                    new_event->dt_stamp_ = QDateTime::fromString(GetProperty(line), Qt::DateFormat::ISODate);
                } else if (line.indexOf("DESCRIPTION") == 0) {
                    new_event->description_ = GetProperty(line);
                } else if (line.indexOf("CATEGORIES") == 0) {
                    new_event->categories_ = GetProperty(line);
                } else if (line.indexOf("RRULE") == 0) {
                    new_event->r_rule_.freq_ = ConvertFrequency(GetSubProperty(line, "FREQ"));
                    new_event->r_rule_.interval_ = GetSubProperty(line, "INTERVAL").toInt();
                    if (new_event->r_rule_.interval_ == 0)
                        new_event->r_rule_.interval_ = 1;
                    new_event->r_rule_.count_ = GetSubProperty(line, "COUNT").toInt();
                    new_event->r_rule_.until_ = QDateTime::fromString(GetSubProperty(line, "UNTIL"), Qt::DateFormat::ISODate);
                } else if (line.indexOf("BEGIN:VALARM") == 0) {
                    new_alarm.Clear();
                    prev_component = current_component;
                    current_component = VALARM;
                } else if (line.indexOf("END:VEVENT") == 0) {
                    if (new_event->uid_.isEmpty())
                        new_event->uid_ = QChar(no_uid++);

                    events_.push_back(new_event);
                    current_component = VCALENDAR;
                }
                break;

            case VALARM:
                if (line.indexOf("ACTION") == 0) {
                    new_alarm.action_ = ConvertAlarmAction(GetProperty(line));
                } else if (line.indexOf("TRIGGER") == 0) {
                    new_alarm.trigger_ = GetProperty(line);
                } else if (line.indexOf("DESCRIPTION") == 0) {
                    new_alarm.description__ = GetProperty(line);
                } else if (line.indexOf("END:VALARM") == 0) {
                    new_event->alarms_->push_back(new_alarm);
                    current_component = prev_component;
                }
                break;
        }
    }
}
