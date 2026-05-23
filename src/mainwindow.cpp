#include "mainwindow.h"
#include "updater.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_updater(new Updater(this))
{
    setWindowTitle(tr("CachyOS Updater"));
    resize(820, 560);

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *header = new QHBoxLayout();
    header->setSpacing(16);
    auto *iconLabel = new QLabel();
    QIcon icon = QIcon::fromTheme("cachyos-updater", QIcon(":/icons/cachyos-updater.svg"));
    iconLabel->setPixmap(icon.pixmap(64, 64));
    header->addWidget(iconLabel);

    auto *titleBox = new QVBoxLayout();
    auto *title = new QLabel(tr("CachyOS Updater"));
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 8);
    titleFont.setBold(true);
    title->setFont(titleFont);
    auto *subtitle = new QLabel(tr("Mantén tu sistema al día con un solo clic."));
    subtitle->setStyleSheet("color: palette(mid);");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    titleBox->addStretch();
    header->addLayout(titleBox);
    header->addStretch();
    root->addLayout(header);

    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    root->addWidget(separator);

    m_updateButton = new QPushButton(tr("Actualizar mi sistema"));
    m_updateButton->setMinimumHeight(56);
    QFont btnFont = m_updateButton->font();
    btnFont.setPointSize(btnFont.pointSize() + 3);
    btnFont.setBold(true);
    m_updateButton->setFont(btnFont);
    m_updateButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #7c3aed;"
        "  color: white;"
        "  border-radius: 10px;"
        "  padding: 12px 24px;"
        "}"
        "QPushButton:hover { background-color: #6d28d9; }"
        "QPushButton:disabled { background-color: #6b7280; }"
    );
    root->addWidget(m_updateButton);

    m_phaseLabel = new QLabel(tr("Listo para actualizar."));
    m_phaseLabel->setStyleSheet("font-weight: bold;");
    root->addWidget(m_phaseLabel);

    m_progress = new QProgressBar();
    m_progress->setRange(0, 0);
    m_progress->setVisible(false);
    m_progress->setTextVisible(false);
    root->addWidget(m_progress);

    m_log = new QPlainTextEdit();
    m_log->setReadOnly(true);
    m_log->setFont(QFont("monospace"));
    m_log->setPlaceholderText(tr("La salida del proceso de actualización aparecerá aquí."));
    m_log->setStyleSheet(
        "QPlainTextEdit {"
        "  background-color: #1e1e2e;"
        "  color: #cdd6f4;"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "}"
    );
    root->addWidget(m_log, 1);

    m_statusLabel = new QLabel(tr("Versión %1").arg(QApplication::applicationVersion()));
    statusBar()->addPermanentWidget(m_statusLabel);
    statusBar()->showMessage(tr("Pulsa el botón para comenzar."));

    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::onUpdateClicked);
    connect(m_updater, &Updater::output, this, &MainWindow::onOutput);
    connect(m_updater, &Updater::phaseChanged, this, &MainWindow::onPhaseChanged);
    connect(m_updater, &Updater::finished, this, &MainWindow::onFinished);
}

void MainWindow::onUpdateClicked()
{
    m_log->clear();
    setBusy(true);
    m_updater->run();
}

void MainWindow::onOutput(const QString &chunk)
{
    appendLog(chunk);
}

void MainWindow::onPhaseChanged(const QString &phase)
{
    m_phaseLabel->setText(phase);
    statusBar()->showMessage(phase);
}

void MainWindow::onFinished(bool ok, const QString &message)
{
    setBusy(false);
    m_phaseLabel->setText(message);
    statusBar()->showMessage(message);
    if (ok) {
        QMessageBox::information(this, tr("Actualización completada"),
            tr("Tu sistema CachyOS está al día.\n\n%1").arg(message));
    } else {
        QMessageBox::warning(this, tr("Algo ha ido mal"),
            tr("La actualización no se completó correctamente.\n\n%1\n\n"
               "Puedes revisar la salida en la ventana para más detalle.")
                .arg(message));
    }
}

void MainWindow::setBusy(bool busy)
{
    m_updateButton->setEnabled(!busy);
    m_updateButton->setText(busy ? tr("Actualizando…") : tr("Actualizar mi sistema"));
    m_progress->setVisible(busy);
}

void MainWindow::appendLog(const QString &text)
{
    QString trimmed = text;
    while (trimmed.endsWith('\n') || trimmed.endsWith('\r')) {
        trimmed.chop(1);
    }
    if (trimmed.isEmpty()) {
        return;
    }
    m_log->appendPlainText(trimmed);
    auto *bar = m_log->verticalScrollBar();
    bar->setValue(bar->maximum());
}
