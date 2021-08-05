#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>
#include <QtNetworkAuth>

#include "calendartypes.h"

class Calendar : public QObject
{
    Q_OBJECT
public:
    explicit Calendar(QObject *parent = nullptr);

    void LoadFromReply(QNetworkReply &reply);

    void AddEvent(Event *NewEvent);
    void DeleteEvent(Event *DeletedEvent);
    void ModifyEvent(Event *ModifiedEvent);
private:
    QString GetProperty(const QString &line) const {
        // if VALUE=DATE or VALUE=DATE-TIME used, then the date is not directly
        // after the property name so we just search for the string after ':'
        QString temp = line.sliced(line.indexOf(':')+1);
        unsigned int length = temp.length();
        if (length > 0 && temp[length-1] == '\r')
            temp.resize(length-1);
        return temp;
    }
    QString GetSubProperty(const QString &line, const char *sub_property) const {
        size_t pos = line.indexOf(sub_property);
        if (pos == -1)
            return "";
        pos += strlen(sub_property) + 1;
        return line.sliced(pos, line.indexOf(';', pos)-pos);
    }
    void FixlineEnd(QString &line, unsigned int length) {
        if (length > 0 && line[length-1] == '\r')
            line.resize(length-1);
        line += "\r\n";
    }

    QList<Event *> Events;
signals:

};

#endif // CALENDAR_H
