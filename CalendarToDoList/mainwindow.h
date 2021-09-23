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

 signals:
  void initialized();

 private slots:
  void initialize();
  void refreshCalendarEvents();

  void synchronize();
  void changeUserServer();
  void changeCalendar();

 private:
  void getExpansion(ICalendarComponent &&original);
  void getUserCalendars();
  void tryGetPrincipal();
  void showEditForm(ICalendarComponent component);

  Ui::MainWindow *ui;

  ICalendar calendar_;
  CalendarClient *client_;

  QPointer<QTimer> timer_;
  bool sync_token_supported_;
  bool ready_user_;
  bool ready_event_;
  bool ready_task_;
};
#endif  // MAINWINDOW_H
