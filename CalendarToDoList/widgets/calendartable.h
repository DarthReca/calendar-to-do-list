#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QGridLayout>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTableWidget>
#include <functional>

#include "calendar_classes/icalendarcomponent.h"
#include "calendartableitem.h"
#include "mainwindow.h"

enum class TimeFrame { kDaily, kWeekly };

class CalendarTable : public QTableWidget {
 public:
  explicit CalendarTable(QWidget* parent = nullptr);
  void init();

  void createTableItem(ICalendarComponent& component, MainWindow* main_window);
  void clearShowingWidgets();

  void removeByHref(const QString& href);
  void removeByUid(const QString& uid);

  auto& getShowingEvents() { return showing_items_; }

  void setVisualMode(TimeFrame new_time_frame, QDate today);
  const TimeFrame& visualMode() { return time_frame_; };

  QPair<QDate, QDate> getDateRange();
  QPair<QDateTime, QDateTime> getDateTimeRange();

 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  void resizeAndMove(CalendarTableItem* widget);
  void clearWidgetList(QList<QPointer<CalendarTableItem>>& list);

  TimeFrame time_frame_;
  QDate today_;

  QHash<QString, QList<QPointer<CalendarTableItem>>> showing_items_;
};

#endif  // CALENDARTABLE_H
