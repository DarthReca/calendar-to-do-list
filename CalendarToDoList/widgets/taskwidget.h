#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QPushButton>
#include <QWidget>

#include "calendar_classes/task.h"
#include "calendar_classes/calendarevent.h"
#include "createeventform.h"

class Taskwidget : public QPushButton {
    Q_OBJECT

public:
    explicit Taskwidget(Task& task, CalendarClient& client,
                         Calendar& calendar, QWidget* parent = nullptr);

    QPointer<Task> task() const;

private:
    QPointer<Task> task_;
    QPointer<CalendarClient> client_;
    QPointer<Calendar> calendar_;
};

#endif // TASKWIDGET_H
