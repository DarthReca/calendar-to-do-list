#include "googleauth.h"
#include <QtNetworkAuth>
#include <QDesktopServices>

GoogleAuth::GoogleAuth(QObject *parent) : QObject(parent)
{
   auto google = new QOAuth2AuthorizationCodeFlow;

   // Permessi richiesti a google. Vedi https://developers.google.com/identity/protocols/oauth2/scopes
   google->setScope("https://www.googleapis.com/auth/calendar https://www.googleapis.com/auth/calendar.events");

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

   auto port = static_cast<quint16>(QUrl(settingsObject["redirect_uris"].toArray()[0].toString()).port());
   auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
   google->setReplyHandler(replyHandler);

   google->grant();
}
