#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include "googleauth/googleauth.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Starting...";
    GoogleAuth(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

}

