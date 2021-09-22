#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCache>
#include <QMainWindow>
#include <QTimer>

#include "calendar_classes/calendar.h"
#include "calendarclient.h"
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

  // void showEventForm(CalendarEvent event);
  // void showTaskForm(Task task);
  void showEditForm(ICalendarComponent component);
  void getUserCalendars();
  void tryGetPrincipal();

 signals:
  void show();
  void initialized();

 private slots:
  void initialize();
  void refresh_calendar_events();
  void on_actionSincronizza_triggered();
  void on_actionCambia_utente_server_triggered();

  void on_actionCambia_calendario_triggered();

 private:
  void getExpansion(ICalendarComponent &&original);

  Ui::MainWindow *ui;

  ICalendar calendar_;
  CalendarClient *client_;

  QPointer<QTimer> timer_;
  bool sync_token_supported_;
  bool readyUser_;
  bool readyEvent;
  bool readyTask;
};
#endif  // MAINWINDOW_H
