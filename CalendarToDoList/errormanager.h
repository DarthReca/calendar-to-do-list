#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <QMessageBox>
#include <QWidget>

#define GENERIC_ERROR 1
#define INITIALIZATION_ERROR 2
#define LOGIC_ERROR 3
#define UNSUPPORTED_ERROR 4
#define NETWORK_ERROR 5

class ErrorManager {
 public:
  static void initializationError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di inizializzazione", description);
    exit(INITIALIZATION_ERROR);
  };
};

#endif  // ERRORMANAGER_H
