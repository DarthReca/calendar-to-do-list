#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QGridLayout>
#include <QObject>
#include <QPointer>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

class CalendarTable : public QWidget {
  Q_OBJECT
 public:
  explicit CalendarTable(QWidget* parent = nullptr);

  void SetDays(QStringList days);
 signals:
 private:
  QPointer<QVBoxLayout> first_level_layout_;
  QPointer<QGridLayout> header_;
  QPointer<QScrollArea> body_;
  QPointer<QVBoxLayout> columns_;
};

#endif  // CALENDARTABLE_H
