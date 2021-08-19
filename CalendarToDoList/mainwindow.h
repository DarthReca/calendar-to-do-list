#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "CalendarClient/calendarclient.h"
#include "calendar_classes/calendar.h"
#include "calendar_classes/calendarevent.h"
#include "widgets/createeventform.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  QList<CalendarEvent> *showing_events() const;
  void setShowing_events(QList<CalendarEvent> *newShowing_events);

 signals:
  void showing_eventsChanged();

 protected:
  void resizeEvent(QResizeEvent *event);

 private slots:
  void on_createEvent_clicked();
  void refresh_calendar_events();

  void on_calendarWidget_clicked(const QDate &date);
  void on_seeIfChanged_clicked();
  void on_showing_events_changed();
  void on_actionSincronizza_triggered();

 private:
  void updateTableToNDays(int n);

  Ui::MainWindow *ui;

  QPointer<CalendarEvent> editing_event_;
  QList<CalendarEvent> *showing_events_;
  QPointer<Calendar> calendar_;

  QPointer<GoogleAuth> auth_;
  QPointer<CalendarClient> client_;

  QPointer<QTimer> timer_;
  QPointer<QTimer> single_shot_timer_;
};
#endif  // MAINWINDOW_H
