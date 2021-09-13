#include "calendartableitem.h"

CalendarTableItem::CalendarTableItem(ICalendarComponent&& component,
                                     QWidget* parent)
    : QPushButton(parent) {
  setComponent(std::move(component));
}

void CalendarTableItem::setComponent(ICalendarComponent&& new_component) {
  component_ = new_component;

  auto start = component_.getStartDateTime();
  auto end = component_.getEndDateTime();

  QString time_string = "";
  if (start.has_value()) time_string += start.value().toString("hh:mm");
  if (!time_string.isEmpty()) time_string += " - ";
  if (end.has_value()) time_string += end.value().toString("hh:mm");

  QString summary = component_.getProperty("SUMMARY").value_or("Senza titolo");
  QString color = component_.type() == "VEVENT" ? "red" : "blue";

  QString text = QString("%1\n%2").arg(summary, time_string);
  if (time_string.length() < 6)
    text = QString("%1 - %2").arg(summary, time_string);
  if (component_.allDay()) text = summary;

  setText(text);
  setStyleSheet(QString("background-color: %1; color: white").arg(color));
}
