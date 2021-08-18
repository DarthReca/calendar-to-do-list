#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "calendar_classes/calendarevent.h"
#include "calendar_classes/calendar.h"
#include "CalendarClient/calendarclient.h"
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

    QList<CalendarEvent> *showing_events() const;
    void setShowing_events(QList<CalendarEvent> *newShowing_events);

    enum Visualization { Week, Day };
    Q_ENUM(Visualization);

signals:
    void showing_eventsChanged();
public slots:
    void refresh_calendar_events();
private slots:
    void on_actionGiorno_triggered();
    void on_actionSettimanale_triggered();

    void on_createEvent_clicked();

    void on_calendarWidget_clicked(const QDate &date);
    void on_seeIfChanged_clicked();
    void on_showing_events_changed();

    void on_actionSincronizza_triggered();

private:
    Ui::MainWindow *ui;

    QPointer<CalendarEvent> editing_event_;
    QList<CalendarEvent>* showing_events_;
    QPointer<Calendar> calendar_;

    QPointer<GoogleAuth> auth_;
    QPointer<CalendarClient> client_;
    Visualization current_visual_;
};
#endif // MAINWINDOW_H
