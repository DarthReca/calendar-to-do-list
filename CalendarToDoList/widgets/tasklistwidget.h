#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QListWidget>
#include <QHash>
#include "calendar_classes/icalendarcomponent.h"

class ComponentListWidget : public QListWidget
{
public:
    ComponentListWidget(QWidget* parent = nullptr);

    void createListWidget(ICalendarComponent&& component);
private:
    QHash<QString, QPair<int, ICalendarComponent>> components_;
};

#endif // TASKLISTWIDGET_H
