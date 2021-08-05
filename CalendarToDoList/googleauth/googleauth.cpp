#include "googleauth.h"
#include <QDesktopServices>
#include <QDomDocument>

GoogleAuth::GoogleAuth(QObject *parent) : QObject(parent)
{
   google = new QOAuth2AuthorizationCodeFlow;

   // Permessi richiesti a google. Vedi https://developers.google.com/identity/protocols/oauth2/scopes
   google->setScope("https://www.googleapis.com/auth/calendar https://www.googleapis.com/auth/calendar.events https://www.googleapis.com/auth/calendar.settings.readonly");

   connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

   // Read authentication data
   QFile data = QFile("auth.json");
   data.open(QFile::ReadOnly);
   auto document = QJsonDocument().fromJson(data.readAll());
   auto settingsObject = document.object()["installed"].toObject();

   google->setAuthorizationUrl(QUrl(settingsObject["auth_uri"].toString()));
   google->setClientIdentifier(settingsObject["client_id"].toString());
   google->setClientIdentifierSharedKey(settingsObject["client_secret"].toString());
   google->setAccessTokenUrl(QUrl(settingsObject["token_uri"].toString()));

   quint16 port = 8001;//static_cast<quint16>(QUrl(settingsObject["redirect_uris"].toArray()[0].toString()).port());
   auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
   google->setReplyHandler(replyHandler);

   google->grant();
   connect(google, &QOAuth2AuthorizationCodeFlow::granted, [this]() {
       qDebug() << google->token();
   });
}

GoogleAuth::~GoogleAuth() {
    delete google;
}
