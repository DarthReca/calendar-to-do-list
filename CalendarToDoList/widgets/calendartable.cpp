#include "calendartable.h"

#include <QHeaderView>
#include <QLabel>
#include <QTime>
#include <QTimer>

CalendarTable::CalendarTable(QWidget *parent) : QTableWidget(parent) {
  showing_events_ = QHash<QString, QList<EventWidgetPointer>>();
  showing_task_ = QHash<QString, QList<TaskWidgetPointer>>();
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

  setVisualMode(TimeFrame::kWeekly, QDate::currentDate());
}

void CalendarTable::resizeEvent(QResizeEvent *event) {
  QTableWidget::resizeEvent(event);
  QPointer<QTimer> timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, [this]() {
    for (auto &widget_list : showing_events_)
      for (auto &widget : widget_list) resizeAndMove<CalendarEvent>(widget);
    for (auto &widget_list : showing_task_)
      for (auto &widget : widget_list) resizeAndMove<Task>(widget);
  });
  timer->start(20);
}

template <class T>
void CalendarTable::resizeAndMove(CalendarTableItem<T> *widget) {
  CalendarEvent event = widget->item();
  QTime start_time = event.startDateTime().time();
  QTime end_time = event.endDateTime().time();
  int day_from_start = today_.daysTo(event.startDateTime().date());

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

void CalendarTable::setVisualMode(TimeFrame new_time_frame, QDate today) {
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
  range.first = today_;
  range.second = today_.addDays(columnCount());
  return range;
}

QPair<QDateTime, QDateTime> CalendarTable::getDateTimeRange() {
  QPair<QDateTime, QDateTime> range;
  auto date_range = getDateRange();
  range.first = QDateTime(date_range.first, QTime(0, 0));
  range.second = QDateTime(date_range.second, QTime(23, 59));
  return range;
}

void CalendarTable::createEventWidget(CalendarEvent &event,
                                      MainWindow *main_window) {
  auto range = getDateRange();
  if (event.startDateTime().date() < range.first ||
      event.endDateTime().date() > range.second)
    return;

  if (showing_events_.contains(event.uid()) &&
      showing_events_[event.uid()][0]->item().eTag() != event.eTag())
    clearWidgetList(showing_events_[event.uid()]);

  auto widget =
      new CalendarTableItem<CalendarEvent>(std::move(event), viewport());
  connect(widget, &QPushButton::clicked, [widget, main_window]() {
    main_window->showEventForm(widget->item());
  });
  showing_events_[event.uid()] += widget;

  resizeAndMove<CalendarEvent>(widget);
}

void CalendarTable::createTaskWidget(Task &task, MainWindow *main_window) {
  auto range = getDateRange();
  if (task.startDateTime().date() < range.first ||
      task.endDateTime().date() > range.second)
    return;
  if (showing_task_.contains(task.uid()) &&
      showing_task_[task.uid()][0]->item().eTag() != task.eTag())
    clearWidgetList(showing_task_[task.uid()]);

  auto widget = new CalendarTableItem<Task>(std::move(task), viewport());
  connect(widget, &CalendarTableItem<Task>::clicked, [main_window, widget]() {
    main_window->showTaskForm(widget->item());
  });
  showing_task_[task.uid()] += widget;

  resizeAndMove<Task>(widget);
}

void CalendarTable::clearShowingWidgets() {
  for (auto &widget_list : showing_events_) clearWidgetList(widget_list);
  showing_events_.clear();
  for (auto &widget_list : showing_task_) clearWidgetList(widget_list);
  showing_task_.clear();
}

void CalendarTable::removeByHref(const QString &href) {
  bool continue_search = true;
  for (auto key_value = showing_events_.keyValueBegin();
       key_value != showing_events_.keyValueEnd(); key_value++) {
    if (key_value->second[0]->item().href() == href) {
      clearWidgetList(key_value->second);
      showing_events_.remove(key_value->first);
      continue_search = false;
      break;
    }
  }
  if (!continue_search) return;
  for (auto key_value = showing_task_.keyValueBegin();
       key_value != showing_task_.keyValueEnd(); key_value++) {
    if (key_value->second[0]->item().href() == href) {
      clearWidgetList(key_value->second);
      showing_task_.remove(key_value->first);
      break;
    }
  }
}

void CalendarTable::removeTaskByUid(const QString &uid) {
  clearWidgetList(showing_task_[uid]);
  showing_task_.remove(uid);
}

void CalendarTable::removeEventByUid(const QString &uid) {
  clearWidgetList(showing_events_[uid]);
  showing_events_.remove(uid);
}

template <class T>
void CalendarTable::clearWidgetList(
    QList<QPointer<CalendarTableItem<T>>> &list) {
  for (auto &widget : list) widget->deleteLater();
  list.clear();
}
