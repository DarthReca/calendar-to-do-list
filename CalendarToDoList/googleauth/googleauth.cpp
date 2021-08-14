#include "googleauth.h"
#include "calendar_classes/calendarevent.h"
#include "CalendarClient/calendarclient.h"
#include <QDesktopServices>
#include <QDomDocument>
#include <QDateTime>

QDomElement cTag;

GoogleAuth::GoogleAuth(QObject *parent) : QObject(parent)
{
   google = new QOAuth2AuthorizationCodeFlow;

   // Permessi richiesti a google. Vedi https://developers.google.com/identity/protocols/oauth2/scopes
   google->setScope("https://www.googleapis.com/auth/calendar https://www.googleapis.com/auth/calendar.events https://www.googleapis.com/auth/calendar.settings.readonly");

   connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

   // Read authentication data
   QFile data = QFile("auth.json");
   if(!data.exists())
    QT_THROW("The file auth.json does not exists");

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

   QFile tokens = QFile("tokens.json");
   if(tokens.exists())
   {
       tokens.open(QFile::ReadOnly);
       auto document = QJsonDocument().fromJson(tokens.readAll()).object();
       QString access_token = document["accessToken"].toString();
       QString refresh_token = document["refreshToken"].toString();

       google->setToken(access_token);
       google->setRefreshToken(refresh_token);
       google->refreshAccessToken();
   }
   else {
    google->grant();
   }

   connect(google, &QOAuth2AuthorizationCodeFlow::error, [this](const QString& error, const QString& desc, const QUrl& uri) {
       qDebug() << "error";
       qDebug() << error;
   });

   connect(google, &QOAuth2AuthorizationCodeFlow::granted, [this]() {
       qDebug() << "Granted";

       QFile tokens = QFile("tokens.json");
       tokens.open(QFile::WriteOnly);
       QJsonObject json_obj;
       json_obj.insert("accessToken", google->token());
       json_obj.insert("refreshToken", google->refreshToken());
       tokens.write(QJsonDocument(json_obj).toJson());
   });
}

GoogleAuth::~GoogleAuth() {
    google->disconnect();
    delete google;
}
