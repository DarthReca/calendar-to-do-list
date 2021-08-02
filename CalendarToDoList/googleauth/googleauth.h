#ifndef GOOGLEAUTH_H
#define GOOGLEAUTH_H

#include <QObject>

class GoogleAuth : public QObject
{
    Q_OBJECT
public:
    explicit GoogleAuth(QObject *parent = nullptr);

signals:

};

#endif // GOOGLEAUTH_H
