#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QGridLayout>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "calendar_classes/calendarevent.h"
#include "eventwidget.h"

enum class TimeFrame { kDaily, kWeekly };

class CalendarTable : public QTableWidget {
  Q_OBJECT
 public:
  explicit CalendarTable(QWidget* parent = nullptr);
  void init();

  /**
   * @brief Create a new widget and show it
   * @param event CalendarEvent showed by the widget
   * @return The created widget
   */
  EventWidget* createEventWidget(CalendarEvent& event);
  /**
   * @brief Remove all widgets
   */
  void clearShowingWidgets();
  /**
   * @brief Remove the widget corresponding to given href
   * @param href HRef of the deleted event
   */
  void removeByHref(const QString& href);

  QHash<QString, QPointer<EventWidget>>& getShowingEvents() {
    return showing_events_;
  }

  void setVisualMode(TimeFrame new_time_frame, QDate today);
  const TimeFrame& visualMode() { return time_frame_; };

  QPair<QDate, QDate> getDateRange();

 signals:
 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  void resizeAndMove(EventWidget* widget);

  TimeFrame time_frame_;
  QDate today_;

  QHash<QString, QPointer<EventWidget>> showing_events_;
};

#endif  // CALENDARTABLE_H
