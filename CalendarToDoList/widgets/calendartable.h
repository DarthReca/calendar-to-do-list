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
#include "taskwidget.h"

enum class TimeFrame { kDaily, kWeekly };

class CalendarTable : public QTableWidget {
 public:
  explicit CalendarTable(QWidget* parent = nullptr);
  void init();

  /**
   * @brief Create a new widget and show it
   * @param event CalendarEvent showed by the widget
   * @return The created widget
   */
  CalendarTableItem<CalendarEvent>* createEventWidget(CalendarEvent& event,
                                                      MainWindow* main_window);
  CalendarTableItem<Task>* createTaskWidget(Task& task,
                                            MainWindow* main_window);
  /**
   * @brief Remove all widgets
   */
  void clearShowingWidgets();
  /**
   * @brief Remove the widget corresponding to given href
   * @param href HRef of the deleted event
   */
  void removeByHref(const QString& href);

  void removeTaskByUid(const QString& uid);
  void removeEventByUid(const QString& uid);

  auto& getShowingEvents() { return showing_events_; }
  auto& getShowingTask() { return showing_task_; }

  void setVisualMode(TimeFrame new_time_frame, QDate today);
  const TimeFrame& visualMode() { return time_frame_; };

  QPair<QDate, QDate> getDateRange();

 signals:
 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  template <class T>
  void resizeAndMove(CalendarTableItem<T>* widget);

  TimeFrame time_frame_;
  QDate today_;

  QHash<QString, QPointer<CalendarTableItem<CalendarEvent>>> showing_events_;
  QHash<QString, QPointer<CalendarTableItem<Task>>> showing_task_;
};

#endif  // CALENDARTABLE_H
