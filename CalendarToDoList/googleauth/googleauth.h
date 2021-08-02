#ifndef GOOGLEAUTH_H
#define GOOGLEAUTH_H

#include <QObject>
#include <QtNetworkAuth>

class GoogleAuth : public QObject
{
    Q_OBJECT
public:
    explicit GoogleAuth(QObject *parent = nullptr);
    ~GoogleAuth();

    QOAuth2AuthorizationCodeFlow* google;
signals:

private:

};

#endif // GOOGLEAUTH_H
