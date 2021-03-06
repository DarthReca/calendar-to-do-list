#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QHash>
#include <QListWidget>

#include "calendar_classes/icalendarcomponent.h"

class ComponentListWidget : public QListWidget {
 public:
  ComponentListWidget(QWidget* parent = nullptr);

  void createListWidget(ICalendarComponent&& component);

  void removeByUid(const QString& uid);
  void removeByHref(const QString& href);
  void clearListWidget();

  ICalendarComponent componentByItem(QListWidgetItem* widget_item);
  const auto& getShowingComponents() { return components_; }

 private:
  QHash<QString, QPair<int, ICalendarComponent>> components_;
};

#endif  // TASKLISTWIDGET_H
