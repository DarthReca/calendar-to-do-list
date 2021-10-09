#include "userform.h"

#include "ui_userform.h"

Userform::Userform(CalendarClient &client, QWidget *parent)
    : QDialog(parent), ui(new Ui::Userform), formClient_(&client) {
  ui->setupUi(this);
}

Userform::~Userform() { delete ui; }

void Userform::on_buttonBox_clicked(QAbstractButton *button) {
  if (button->text() == "OK") {
    QUrl host = ui->lineEdit_Host->text();
    QString username = ui->lineEdit_Username->text();
    QString password = ui->lineEdit_Password->text();
    QString urlStr = ui->lineEdit_Principal->text();

    if (host.isEmpty() || username.isEmpty() || password.isEmpty()) {
      QMessageBox::critical(this, "Errore", "I campi sono tutti obbligatori");
      exit(-1);
    } else {
      formClient_->setHost(host);
      QByteArray cred = QString(username + ":" + password).toUtf8().toBase64();
      formClient_->setCredentials(cred);
      QUrl url = QUrl(urlStr);
      formClient_->setEndpoint(url);
      formClient_->setPrincipal(urlStr);

      QJsonObject auth_json;
      auth_json.insert("host", host.toString());
      auth_json.insert("username", username);
      auth_json.insert("password", password);
      auth_json.insert("principal",urlStr);
      QJsonDocument doc(auth_json);
      QFile jsonFile("auth.json");
      jsonFile.open(QFile::ReadWrite);
      jsonFile.write(doc.toJson());
    }
  } else {
    exit(0);
  }
}
