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
    Q_PROPERTY(QList<CalendarEvent *> showing_events READ showing_events WRITE setShowing_events NOTIFY showing_eventsChanged)

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    const QList<CalendarEvent *> &showing_events() const;
    void setShowing_events(const QList<CalendarEvent *> &newShowing_events);

signals:
    void showing_eventsChanged();

private slots:
    void on_actionLogin_triggered();
    void on_actionGiorno_triggered();
    void on_actionSettimanale_triggered();

    void on_createEvent_clicked();

    void on_receiveChanges_clicked();

    void on_calendarWidget_clicked(const QDate &date);
    void on_seeIfChanged_clicked();

private:
    Ui::MainWindow *ui;
    CalendarEvent* editing_event_;
    QList<CalendarEvent*> showing_events_;
};
#endif // MAINWINDOW_H
