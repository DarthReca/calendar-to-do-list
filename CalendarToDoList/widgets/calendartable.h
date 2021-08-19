#ifndef CALENDARTABLE_H
#define CALENDARTABLE_H

#include <QObject>
#include <QTableWidget>
#include <QWidget>

class CalendarTable : public QTableWidget {
  Q_OBJECT
 public:
  explicit CalendarTable(QWidget *parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent *event);
 signals:
};

#endif  // CALENDARTABLE_H
