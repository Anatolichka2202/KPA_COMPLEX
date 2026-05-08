#include "mainwindow.h"
#include <QPropertyAnimation>
#include <QDebug>
#include <QPushButton>
#include <QStatusBar>
#include <QMessageBox>
#include <QProcess>
#include "core/types.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("КПА - Тест блока (0)");
    resize(600, 500);

    // Создаём модель и виджет блока 0
    m_model = new BlockModel(0, this);
    m_block = new Block(m_model, this);
    setCentralWidget(m_block);

    connect(m_block, &Block::setpointChanged, this, &MainWindow::onSetpoint);
    connect(m_block, &Block::fireChannel, this, &MainWindow::onFireChannel);
    connect(m_block, &Block::requestResize, this, &MainWindow::onResizeRequest);
    connect(m_model, &BlockModel::pyroFired, this, &MainWindow::onPyroFired);

    // Кнопки управления
    QToolBar *toolbar = addToolBar("Управление");
    m_pingButton = new QPushButton("Тест связи", this);
    m_startStopButton = new QPushButton("Старт опроса", this);
    m_startStopButton->setCheckable(true);
    toolbar->addWidget(m_pingButton);
    toolbar->addWidget(m_startStopButton);
    connect(m_pingButton, &QPushButton::clicked, this, &MainWindow::onPingClicked);
    connect(m_startStopButton, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);

    // Таймер для чтения данных из очереди Master
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateFromMaster);
    m_updateTimer->start(33); // ~30 fps
}

MainWindow::~MainWindow()
{
    m_updateTimer->stop();
}

void MainWindow::updateFromMaster()
{
    bkd::core::TickData data;
    while (bkd::core::g_masterToGui.pop(data)) {
        m_model->updateFromTickData(data);
    }
}

void MainWindow::onSetpoint(int gaugeIndex, double value)
{
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_DRIVE_ANGLES;
    cmd.block = 0;
    double cur1 = m_block->currentAngle1();
    double cur2 = m_block->currentAngle2();
    if (gaugeIndex == 0) cur1 = value;
    else cur2 = value;
    cmd.drive.angles[0] = static_cast<uint16_t>((cur1 + 140.0) / 280.0 * 65535.0); // обратное преобразование
    cmd.drive.angles[1] = static_cast<uint16_t>((cur2 + 140.0) / 280.0 * 65535.0);
    cmd.drive.angles[2] = 0;
    cmd.drive.angles[3] = 0;
    bkd::core::g_guiToMaster.push(cmd);
}

void MainWindow::onFireChannel(int channel)
{
    // Только отправляем команду в модель, модель сама отправит в Master
    m_model->onPyroRequested(channel);
}

void MainWindow::onResizeRequest(bool expand)
{
    animateResize(expand);
}

void MainWindow::onPyroFired(int channel, qint64 requestTime, qint64 confirmTime)
{
    qint64 delay = (requestTime == 0) ? -1 : (confirmTime - requestTime);
    statusBar()->showMessage(QString("Пиро %1 сработало, задержка %2 мс").arg(channel).arg(delay), 2000);
}

void MainWindow::animateResize(bool expand)
{
    int delta = expand ? 300 : -300;
    QPropertyAnimation *anim = new QPropertyAnimation(this, "minimumHeight");
    anim->setDuration(300);
    anim->setEndValue(this->minimumHeight() + delta);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    QPropertyAnimation *animMax = new QPropertyAnimation(this, "maximumHeight");
    animMax->setDuration(300);
    animMax->setEndValue(this->maximumHeight() + delta);
    animMax->start(QAbstractAnimation::DeleteWhenStopped);
}

bool MainWindow::pingHost(const QString &host, int timeoutMs)
{
#ifdef Q_OS_WIN
    QStringList args = { "-n", "1", "-w", QString::number(timeoutMs), host };
#else
    QStringList args = { "-c", "1", "-W", QString::number(timeoutMs/1000), host };
#endif
    QProcess ping;
    ping.start("ping", args);
    return ping.waitForFinished(timeoutMs + 1000) && ping.exitCode() == 0;
}

void MainWindow::onPingClicked()
{
    QString host = bkd::core::YLS_IP; // из types.h
    if (pingHost(host, 2000))
        statusBar()->showMessage("ЯЛС доступен", 2000);
    else
        statusBar()->showMessage("ЯЛС НЕ ДОСТУПЕН", 2000);
}

void MainWindow::onStartStopClicked()
{
    m_pollingActive = m_startStopButton->isChecked();
    bkd::core::GuiCommand cmd;
    cmd.type = m_pollingActive ? bkd::core::GuiCommand::START_POLLING : bkd::core::GuiCommand::STOP_POLLING;
    cmd.block = 0;
    bkd::core::g_guiToMaster.push(cmd);
    m_startStopButton->setText(m_pollingActive ? "Стоп опроса" : "Старт опроса");
}
