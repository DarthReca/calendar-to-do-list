#include "calendartable.h"

#include <QHeaderView>
#include <QLabel>
#include <QTime>
#include <QTimer>

CalendarTable::CalendarTable(QWidget *parent) : QTableWidget(parent) {
  showing_events_ = QHash<QString, QPointer<EventWidget>>();
}

void CalendarTable::init() {
  // Stretch
  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  // Vertical Header
  setRowCount(24);
  setVerticalHeaderItem(0, new QTableWidgetItem(""));
  for (QTime t(0, 0); t.hour() < 23; t = t.addSecs(60 * 60))
    setVerticalHeaderItem(
        t.hour() + 1,
        new QTableWidgetItem(t.toString("hh:mm") + " - " +
                             t.addSecs(60 * 60).toString("hh:mm")));
  setVerticalHeaderItem(23, new QTableWidgetItem("23:00 - 00:00"));

  setVisualMode(TimeFrame::kWeekly, QDateTime::currentDateTime());
}

void CalendarTable::resizeEvent(QResizeEvent *event) {
  QTableWidget::resizeEvent(event);
  QTimer *timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, [this]() {
    for (auto &widget : showing_events_) resizeAndMove(widget);
  });
  timer->start(20);
}

void CalendarTable::resizeAndMove(EventWidget *widget) {
  CalendarEvent event = widget->event();
  QTime start_time = event.startDateTime().time();
  QTime end_time = event.endDateTime().time();
  int day_from_start = today_.date().daysTo(event.startDateTime().date());

  int days_long = event.startDateTime().daysTo(event.endDateTime());

  int x_pos = 0;
  for (int i = 0; i < day_from_start; i++) x_pos += columnWidth(i);

  int start_y_pos = 0;
  for (int i = 0; i <= start_time.hour(); i++) start_y_pos += rowHeight(i);
  start_y_pos += (start_time.minute() / 60.0) * rowHeight(start_time.hour());

  int end_y_pos = 0;
  for (int i = 0; i <= end_time.hour(); i++) end_y_pos += rowHeight(i);
  end_y_pos += (end_time.minute() / 60.0) * rowHeight(end_time.hour());

  int width = columnWidth(day_from_start);
  int height = end_y_pos - start_y_pos;
  if (height == 0) height = rowHeight(start_time.hour()) / 2;

  scrollToTop();
  if (days_long == 0 && !event.all_day()) {
    widget->resize(width, height);
    widget->move(x_pos, start_y_pos);
  } else {
    widget->move(x_pos, 0);
    widget->resize((days_long + 1) * width, rowHeight(0));
  }
  widget->show();
}

void CalendarTable::setVisualMode(TimeFrame new_time_frame, QDateTime today) {
  time_frame_ = new_time_frame;
  today_ = today;
  switch (time_frame_) {
    case TimeFrame::kDaily:
      setColumnCount(1);
      break;
    default:
      setColumnCount(7);
      break;
  }
  for (int i = 0; i < columnCount(); i++)
    setHorizontalHeaderItem(
        i, new QTableWidgetItem(today.addDays(i).toString("ddd\ndd")));
}

QPair<QDate, QDate> CalendarTable::getDateRange() {
  QPair<QDate, QDate> range;
  range.first = today_.date();
  range.second = today_.addDays(columnCount()).date();
  return range;
}

EventWidget &CalendarTable::createEventWidget(CalendarEvent &event) {
  // ACTUAL ASSUMPTION NO RECURRENCY
  QPointer<EventWidget> widget;
  if (showing_events_.contains(event.uid())) {
    widget = showing_events_[event.uid()];
    widget->setEvent(std::move(event));
  } else {
    widget = new EventWidget(std::move(event), viewport());
    resizeAndMove(widget);
    showing_events_[event.uid()] = widget;
  }

  return *widget;
}

void CalendarTable::clearShowingWidgets() {
  for (auto &widget : showing_events_) widget->deleteLater();
  showing_events_.clear();
}
