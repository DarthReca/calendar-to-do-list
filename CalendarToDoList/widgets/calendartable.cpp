#include "calendartable.h"

CalendarTable::CalendarTable(QWidget *parent) : QTableWidget(parent) {
  setEditTriggers(EditTrigger::NoEditTriggers);
  setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  setRowCount(25);
}

void CalendarTable::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}
