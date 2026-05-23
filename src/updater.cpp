#include "updater.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace {
constexpr const char *kPolkitAction = "org.cachyos.updater.run";

QStringList candidateHelperPaths()
{
    QStringList list;
    list << QStringLiteral("/usr/libexec/cachyos-updater/cachyos-updater-helper");
    list << QStringLiteral("/usr/lib/cachyos-updater/cachyos-updater-helper");
    list << QStringLiteral("/usr/local/libexec/cachyos-updater/cachyos-updater-helper");
    list << QStringLiteral("/usr/local/lib/cachyos-updater/cachyos-updater-helper");

    const QString appDir = QCoreApplication::applicationDirPath();
    list << QDir(appDir).filePath("../libexec/cachyos-updater/cachyos-updater-helper");
    list << QDir(appDir).filePath("../lib/cachyos-updater/cachyos-updater-helper");
    list << QDir(appDir).filePath("cachyos-updater-helper");
    list << QDir(appDir).filePath("../packaging/cachyos-updater-helper.sh");
    return list;
}
}

Updater::Updater(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_process, &QProcess::readyReadStandardOutput,
        this, &Updater::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError,
        this, &Updater::onReadyReadStderr);
    connect(m_process, &QProcess::finished, this, &Updater::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Updater::onProcessError);
}

bool Updater::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

QString Updater::resolveHelperPath() const
{
    for (const QString &candidate : candidateHelperPaths()) {
        QFileInfo info(candidate);
        if (info.exists() && info.isExecutable()) {
            return info.absoluteFilePath();
        }
    }
    return {};
}

void Updater::run()
{
    if (isRunning()) {
        emit output(tr("Ya hay una actualización en curso.\n"));
        return;
    }

    const QString helper = resolveHelperPath();
    if (helper.isEmpty()) {
        emit phaseChanged(tr("No se encontró el ayudante de actualización."));
        emit finished(false,
            tr("No se localizó el script auxiliar. Reinstala el paquete."));
        return;
    }

    const QString pkexec = QStandardPaths::findExecutable("pkexec");
    if (pkexec.isEmpty()) {
        emit phaseChanged(tr("Falta pkexec en el sistema."));
        emit finished(false,
            tr("No se encontró pkexec (polkit). Instala polkit para continuar."));
        return;
    }

    emit phaseChanged(tr("Solicitando permisos al sistema…"));
    emit output(tr("► Lanzando %1 con pkexec…\n").arg(QFileInfo(helper).fileName()));

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONUNBUFFERED", "1");
    env.insert("LC_ALL", env.value("LC_ALL", "C.UTF-8"));
    m_process->setProcessEnvironment(env);

    QStringList args;
    args << "--disable-internal-agent" << helper;
    m_process->start(pkexec, args);
}

void Updater::onReadyReadStdout()
{
    const QByteArray data = m_process->readAllStandardOutput();
    const QString text = QString::fromUtf8(data);
    if (text.contains("==> phase:")) {
        for (const QString &line : text.split('\n')) {
            int idx = line.indexOf("==> phase:");
            if (idx >= 0) {
                emit phaseChanged(line.mid(idx + 10).trimmed());
            } else if (!line.trimmed().isEmpty()) {
                emit output(line + "\n");
            }
        }
    } else {
        emit output(text);
    }
}

void Updater::onReadyReadStderr()
{
    const QByteArray data = m_process->readAllStandardError();
    emit output(QString::fromUtf8(data));
}

void Updater::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const bool ok = (status == QProcess::NormalExit && exitCode == 0);
    if (ok) {
        emit finished(true, tr("Sistema actualizado correctamente."));
    } else if (exitCode == 126 || exitCode == 127) {
        emit finished(false,
            tr("Permiso denegado o ayudante no ejecutable (código %1).")
                .arg(exitCode));
    } else {
        emit finished(false,
            tr("El proceso terminó con código %1.").arg(exitCode));
    }
    Q_UNUSED(kPolkitAction);
}

void Updater::onProcessError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        emit finished(false, tr("No se pudo iniciar pkexec."));
    }
}
