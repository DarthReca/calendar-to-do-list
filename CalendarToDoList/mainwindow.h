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

  void showEventForm(CalendarEvent event);
  void showTaskForm(Task task);

 signals:
  void show();
  void initialized();

 private slots:
  void refresh_calendar_events();
  void on_actionSincronizza_triggered();
  void compareElements(QNetworkReply &reply, QHash<QString, QString> &mapTmp);
  void fetchChangedElements(QHash<QString, QString> &mapTmp);

 private:
  Ui::MainWindow *ui;

  QPointer<CalendarEvent> editing_event_;
  ICalendar calendar_;
  QPointer<CalendarClient> client_;

  QPointer<QTimer> timer_;
  bool sync_token_supported_;
  bool readyEvent;
  bool readyTask;
};
#endif  // MAINWINDOW_H
