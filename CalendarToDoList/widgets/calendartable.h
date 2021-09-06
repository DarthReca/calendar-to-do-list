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

  EventWidget& createEventWidget(CalendarEvent& event);
  void clearShowingWidgets();

  void setVisualMode(TimeFrame new_time_frame, QDateTime today);
  const TimeFrame& visualMode() { return time_frame_; };

 signals:
 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  void resizeAndMove(EventWidget* widget);

  TimeFrame time_frame_;
  QDateTime today_;

  QList<QPointer<EventWidget>> showing_events_;
};

#endif  // CALENDARTABLE_H
