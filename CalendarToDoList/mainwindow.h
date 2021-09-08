#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCache>
#include <QMainWindow>
#include <QTimer>

#include "CalendarClient/calendarclient.h"
#include "calendar_classes/calendar.h"
#include "calendar_classes/calendarevent.h"
#include "calendar_classes/task.h"
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

  QList<CalendarEvent> showing_events() const;
  void setShowing_events(QList<CalendarEvent> newShowing_events);

  const QList<Task> &showing_tasks() const;
  void setShowing_tasks(const QList<Task> &newShowing_tasks);

 signals:
  void show();
  void initialized();

 private slots:
  void refresh_calendar_events();

  void on_calendarWidget_clicked(const QDate &date);
  void on_showing_events_changed();
  void on_showing_tasks_changed();
  void on_actionSincronizza_triggered();
  void compareElements(QNetworkReply &reply, QHash<QString, QString> &mapTmp);
  void fetchChangedElements(QHash<QString, QString> &mapTmp);

 private:
  void updateTableToNDays(int n);
  void on_request_editing_form(CalendarEvent event, bool isEvent = true);

  Ui::MainWindow *ui;

  QPointer<CalendarEvent> editing_event_;

  QList<CalendarEvent> showing_events_;
  QList<Task> showing_tasks_;
  QPointer<Calendar> calendar_;

  QPointer<CalendarClient> client_;

  QPointer<QTimer> timer_;
  bool sync_token_supported_;
};
#endif  // MAINWINDOW_H
