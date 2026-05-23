#pragma once

#include <QObject>
#include <QProcess>

class Updater : public QObject
{
    Q_OBJECT

public:
    explicit Updater(QObject *parent = nullptr);

    void run();
    bool isRunning() const;

signals:
    void output(const QString &chunk);
    void phaseChanged(const QString &phase);
    void finished(bool ok, const QString &message);

private slots:
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);

private:
    QString resolveHelperPath() const;

    QProcess *m_process = nullptr;
};
