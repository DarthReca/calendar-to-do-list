#include "componentlistwidget.h"

ComponentListWidget::ComponentListWidget(QWidget *parent)
    : QListWidget(parent) {}

void ComponentListWidget::createListWidget(ICalendarComponent &&component) {
  QString uid = component.getUID();

  if (!components_.contains(uid)) {
    new QListWidgetItem(this);
    components_[uid].first = count() - 1;
  }

  QListWidgetItem *widget = item(components_[uid].first);
  widget->setText(component.getProperty("SUMMARY").value_or("Senza titolo"));
  widget->setData(Qt::UserRole, uid);
  components_[uid].second = std::move(component);
}

void ComponentListWidget::removeByUid(const QString &uid) {
  if (!components_.contains(uid)) return;

  takeItem(components_[uid].first);
  components_.remove(uid);
}

void ComponentListWidget::removeByHref(const QString &href) {
  for (auto &comp : components_) {
    if (comp.second.href() == href) {
      removeByUid(comp.second.getUID());
      break;
    }
  }
}

void ComponentListWidget::clearListWidget() {
  clear();
  components_.clear();
}

ICalendarComponent ComponentListWidget::componentByItem(
    QListWidgetItem *widget_item) {
  QString uid = widget_item->data(Qt::UserRole).toString();
  return components_[uid].second;
}
