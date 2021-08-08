#ifndef CALENDARTYPES_H
#define CALENDARTYPES_H

#include <QDate>

typedef enum { VCALENDAR, VEVENT, VALARM } Component;
typedef enum { DISPLAY=0, PROCEDURE, AUDIO, EMAIL } AlarmAction;
typedef enum { YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, WEEK } TimeUnit;

inline TimeUnit ConvertFrequency(QString name) {
    if (name == "SECONDLY")
        return SECOND;
    if (name == "MINUTELY")
        return MINUTE;
    if (name == "HOURLY")
        return HOUR;
    if (name == "DAILY")
        return DAY;
    if (name == "WEEKLY")
        return WEEK;
    if (name == "MONTHLY")
        return MONTH;

    return YEAR;
}

inline AlarmAction ConvertAlarmAction(QString name) {
    if (name == "AUDIO")
        return AUDIO;
    if (name == "PROCEDURE")
        return PROCEDURE;
    if (name == "EMAIL")
        return EMAIL;

    return DISPLAY;
}

class Recurrence {
public:
    Recurrence(): freq_(YEAR), interval_(0), count_(0) {}
    bool IsEmpty() const { return (interval_ == 0); }
    void Clear() { interval_ = 0; }

    TimeUnit freq_;
    unsigned short interval_, count_;
    QDateTime until_;
};

class AlarmTrigger {
public:
    AlarmTrigger(): before_(true), value_(0), unit_(MINUTE) {}
    AlarmTrigger &operator=(const QString &text)
    {
        char unit_char;
        // 1 because always at least 'P' before the value
        short i = 1;

        if (text[0] == '-')
            before_ = true;
        while (!text[i].isDigit())
            ++i;

        unit_char = text.back().toLatin1();
        value_ = text.sliced(i, text.length() - 1).toShort();
        //sscanf(c_str+i, "%hd%c", (short *)&value_, &unit_char);

        switch (unit_char) {
            case 'H': unit_ = HOUR; break;
            case 'D': unit_ = DAY; break;
            case 'W': unit_ = WEEK; break;
            default: unit_ = MINUTE; break;
        }

        return *this;
    }
private:
    bool before_;
    unsigned short value_;
    TimeUnit unit_;
};

class Alarm {
public:
    Alarm(): active_(false), action_(DISPLAY) {}
    void Clear() {
        description__.clear();
    }

    bool active_;
    AlarmAction action_;
    AlarmTrigger trigger_;
    QString description__;
};

class Event {
public:
    Event(): alarms_(new QList<Alarm>), RecurrenceNo(0), baseEvent(this) {}
    Event(const Event &base):
        uid_(base.uid_),
        summary_(base.summary_),
        description_(base.description_),
        categories_(base.categories_),
        dt_stamp_(base.dt_stamp_),
        dt_start_(base.dt_start_),
        dt_end_(base.dt_end_),
        r_rule_(base.r_rule_),
        alarms_(base.alarms_),
        RecurrenceNo(base.RecurrenceNo)
    {
        baseEvent = base.baseEvent == (Event *)&base ? (Event *)&base : base.baseEvent;
    }
    ~Event() {
        if (baseEvent == this)
            delete alarms_;
    }
    bool HasAlarm(const QDate &From, const QDate &To);

    QString uid_, summary_, description_, categories_;
    QDateTime dt_stamp_, dt_start_, dt_end_;
    Recurrence r_rule_;
    QList<Alarm> *alarms_;
    unsigned short RecurrenceNo;
    Event *baseEvent;
};

class EventsCriteria {
public:
    EventsCriteria(): all_events_(false), include_recurrent_(true) {}

    QDateTime from_, to_;
    bool all_events_, include_recurrent_;
};

#endif // CALENDARTYPES_H
