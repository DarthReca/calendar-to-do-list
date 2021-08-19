#ifndef GOOGLEAUTH_H
#define GOOGLEAUTH_H

#include <QFuture>
#include <QObject>
#include <QtNetworkAuth>

class GoogleAuth : public QObject {
  Q_OBJECT
 public:
  explicit GoogleAuth(QObject *parent = nullptr);
  ~GoogleAuth();

  QPointer<QOAuth2AuthorizationCodeFlow> google;
};

#endif  // GOOGLEAUTH_H
