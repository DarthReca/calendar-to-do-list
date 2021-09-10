#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/calendarevent.h"
#include "createeventform.h"

template <class T>
class CalendarTableItem : public QPushButton {
 public:
  explicit CalendarTableItem(T&& event, QWidget* parent = nullptr);

  void setItem(T&& new_event);
  T& item() { return item_; };

 private:
  T item_;
};

template <class T>
CalendarTableItem<T>::CalendarTableItem(T&& event, QWidget* parent)
    : QPushButton(parent) {
  setItem(std::move(event));
}

template <class T>
void CalendarTableItem<T>::setItem(T&& event) {
  item_ = event;
  QString start_time = item_.startDateTime().toString("hh:mm");
  QString end_time = item_.endDateTime().toString("hh:mm");
  QString text =
      QString("%1\n%2 - %3").arg(item_.summary(), start_time, end_time);
  if (start_time == end_time)
    text = QString("%1\t%2").arg(item_.summary(), start_time);
  if (item_.all_day()) text = item_.summary();

  setText(text);
  setStyleSheet(
      QString("background-color: %1; color: white").arg(item_.getColor()));
}

#endif  // EVENTWIDGET_H
