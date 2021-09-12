#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QGridLayout>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTableWidget>
#include <functional>

#include "calendar_classes/calendarevent.h"
#include "eventwidget.h"
#include "mainwindow.h"

using EventWidgetPointer = QPointer<CalendarTableItem<CalendarEvent>>;
using TaskWidgetPointer = QPointer<CalendarTableItem<Task>>;

enum class TimeFrame { kDaily, kWeekly };

class CalendarTable : public QTableWidget {
 public:
  explicit CalendarTable(QWidget* parent = nullptr);
  void init();

  /**
   * @brief Create a new widget and show it
   * @param event CalendarEvent showed by the widget
   * @param main_window It musts contain a function to connect
   * @return the created widget
   */
  void createEventWidget(CalendarEvent& event, MainWindow* main_window);
  void createTaskWidget(Task& task, MainWindow* main_window);
  /**
   * @brief Remove all widgets
   */
  void clearShowingWidgets();

  void removeByHref(const QString& href);
  void removeTaskByUid(const QString& uid);
  void removeEventByUid(const QString& uid);

  auto& getShowingEvents() { return showing_events_; }
  auto& getShowingTask() { return showing_task_; }

  void setVisualMode(TimeFrame new_time_frame, QDate today);
  const TimeFrame& visualMode() { return time_frame_; };

  QPair<QDate, QDate> getDateRange();

 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  template <class T>
  void resizeAndMove(CalendarTableItem<T>* widget);
  template <class T>
  void clearWidgetList(QList<QPointer<CalendarTableItem<T>>>& list);

  TimeFrame time_frame_;
  QDate today_;

  QHash<QString, QList<EventWidgetPointer>> showing_events_;
  QHash<QString, QList<TaskWidgetPointer>> showing_task_;
};

#endif  // CALENDARTABLE_H
