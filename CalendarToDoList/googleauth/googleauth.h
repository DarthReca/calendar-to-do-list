#ifndef GOOGLEAUTH_H
#define GOOGLEAUTH_H

#include <QObject>
#include <QtNetworkAuth>
#include <QFuture>

class GoogleAuth : public QObject
{
    Q_OBJECT
public:
    explicit GoogleAuth(QObject *parent = nullptr);
    ~GoogleAuth();

    QOAuth2AuthorizationCodeFlow* google;
};

#endif // GOOGLEAUTH_H
