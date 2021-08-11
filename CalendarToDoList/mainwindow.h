#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "calendar_classes/calendarevent.h"
#include "widgets/createeventform.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

private slots:
    void on_actionLogin_triggered();
    void on_request_event_ui(CalendarEvent event);
    void on_addButton_clicked();

    void on_pushButton_clicked();

    void on_creaEvento_clicked();

private:
    Ui::MainWindow *ui;
    CalendarEvent* editing_event_;
};
#endif // MAINWINDOW_H
