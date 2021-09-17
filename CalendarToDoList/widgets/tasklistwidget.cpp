#include "tasklistwidget.h"

ComponentListWidget::ComponentListWidget(QWidget* parent) :
    QListWidget(parent),
    components_(QHash<QString, QPair<int, ICalendarComponent>>())
{}

void ComponentListWidget::createListWidget(ICalendarComponent &&component)
{
    QString uid = component.getUID();

    if(!components_.contains(uid))
    {
       new QListWidgetItem(this);
       components_[uid].first = count() - 1;
    }

    item(components_[uid].first)->setText(component.getProperty("SUMMARY").value_or("Senza titolo"));
    components_[uid].second = std::move(component);
}
