#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"
#include <QLabel>

using namespace std;

GoogleAuth* auth;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Starting...\n";
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginButton_clicked()
{
    auth = new GoogleAuth(this);

    connect(auth->google, &QOAuth2AuthorizationCodeFlow::granted, [this]() {
        findChild<QLabel*>("connectionLabel")->setText("connected");
    });
}

