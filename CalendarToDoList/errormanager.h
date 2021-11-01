#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <QMessageBox>
#include <QWidget>

#define GENERIC_ERROR 1
#define INITIALIZATION_ERROR 2
#define LOGIC_ERROR 3
#define UNSUPPORTED_ERROR 4
#define NETWORK_ERROR 5
#define CREATION_ERROR 6
#define DELETION_ERROR 7
#define SYNC_ERROR 8

class ErrorManager {
 public:
  static void initializationError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di inizializzazione", description);
    exit(INITIALIZATION_ERROR);
  };
  static void logicError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore", description);
    exit(LOGIC_ERROR);
  };
  static void supportError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di compatibilità", description);
    exit(UNSUPPORTED_ERROR);
  };
  static void networkError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di connettività di rete", description);
    exit(NETWORK_ERROR);
  };
  static void creationError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di creazione", description);
    exit(CREATION_ERROR);
  };
  static void deletionError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di cancellazione", description);
    exit(DELETION_ERROR);
  };
  static void synchronizationError(QWidget* parent, const QString& description) {
    QMessageBox::critical(parent, "Errore di sincronizzazione", description);
    exit(SYNC_ERROR);
  };

};

#endif  // ERRORMANAGER_H
