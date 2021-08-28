#include "calendartable.h"

#include <QLabel>
#include <QTime>

CalendarTable::CalendarTable(QWidget* parent) : QWidget(parent) {
  first_level_layout_ = new QVBoxLayout(this);
  header_ = new QGridLayout();
  body_ = new QScrollArea();
  columns_ = new QVBoxLayout(body_);

  first_level_layout_->addLayout(header_);
  first_level_layout_->addWidget(body_);

  // Add hours
  for (QTime t = QTime(0, 0); t.hour() < 23; t = t.addSecs(60 * 60))
    columns_->addWidget(new QLabel(t.toString("hh:mm")));
  columns_->addWidget(new QLabel("23:00"));
}

void CalendarTable::SetDays(QStringList days) {
  for (auto child : header_->children()) child->deleteLater();
  // First column empty
  header_->addWidget(new QLabel(""), 0, 0);
  header_->addWidget(new QLabel(""), 1, 0);
  for (int i = 0; i < days.size(); i++) {
    header_->addWidget(new QLabel(days[i]), 0, i + 1);
  }
}
