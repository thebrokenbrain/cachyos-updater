#pragma once

#include <QMainWindow>

class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class Updater;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onUpdateClicked();
    void onOutput(const QString &chunk);
    void onPhaseChanged(const QString &phase);
    void onFinished(bool ok, const QString &message);

private:
    void setBusy(bool busy);
    void appendLog(const QString &text);

    Updater *m_updater = nullptr;
    QPushButton *m_updateButton = nullptr;
    QPlainTextEdit *m_log = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_phaseLabel = nullptr;
};
