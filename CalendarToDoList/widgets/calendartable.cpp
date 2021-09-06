#include "calendartable.h"

#include <QHeaderView>
#include <QLabel>
#include <QTime>

CalendarTable::CalendarTable(QWidget *parent) : QTableWidget(parent) {
  showing_events_ = QList<QPointer<EventWidget>>();
}

void CalendarTable::Init() {
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

  SetVisualMode(TimeFrame::kWeekly, QDateTime::currentDateTime());
}

void CalendarTable::resizeEvent(QResizeEvent *event) {
  for (auto &widget : showing_events_) ResizeAndMove(widget);
  QTableWidget::resizeEvent(event);
}

void CalendarTable::ResizeAndMove(EventWidget *widget) {
  CalendarEvent event = widget->GetEvent().data();
  QTime start_time = event.getStartDateTime().time();
  int day_from_start = today_.date().daysTo(event.getStartDateTime().date());

  int days_long = event.getStartDateTime().daysTo(event.getEndDateTime());
  int time_long = event.getEndDateTime().time().hour() - start_time.hour();

  int x_pos = 0;
  for (int i = 0; i < day_from_start; i++) x_pos += columnWidth(i);

  int y_pos = 0;
  for (int i = 0; i <= start_time.hour() + 1; i++) y_pos += rowHeight(i);
  // y_pos += (start_time.minute() / 60.0) * rowHeight(start_time.hour());

  int width = columnWidth(day_from_start);
  int heigth = rowHeight(start_time.hour());

  scrollToTop();
  if (days_long == 0 && !event.all_day()) {
    widget->resize(width, heigth * time_long);
    widget->move(x_pos, y_pos);
  } else {
    widget->move(x_pos, 0);
    widget->resize((days_long + 1) * width, heigth);
  }
  widget->show();
}

void CalendarTable::SetVisualMode(TimeFrame new_time_frame, QDateTime today) {
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

EventWidget &CalendarTable::CreateEventWidget(CalendarEvent &event) {
  EventWidget *widget = new EventWidget(event, viewport());
  ResizeAndMove(widget);

  showing_events_ += widget;

  return *widget;
}

void CalendarTable::ClearShowingWidgets() {
  for (auto widget : showing_events_) widget->deleteLater();
  showing_events_.clear();
}
