#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QGridLayout>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTableWidget>

#include "calendar_classes/calendarevent.h"
#include "eventwidget.h"
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
  CalendarTableItem<CalendarEvent>* createEventWidget(CalendarEvent& event);
  CalendarTableItem<Task>* createTaskWidget(Task& task);
  /**
   * @brief Remove all widgets
   */
  void clearShowingWidgets();
  /**
   * @brief Remove the widget corresponding to given href
   * @param href HRef of the deleted event
   */
  void removeByHref(const QString& href);

  auto& getShowingEvents() { return showing_events_; }
  auto& getShowingTaks() { return showing_task_; }

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
