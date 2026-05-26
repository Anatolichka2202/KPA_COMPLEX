#include "mainwindow.h"
#include <QPropertyAnimation>
#include <QDebug>
#include <QPushButton>
#include <QStatusBar>
#include <QMessageBox>
#include <QProcess>
#include <QToolBar>

#include "core/master.h"
#include "core/types.h"
#include "network/real_yls_network.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("КПА - Тест блока (0)");
    resize(600, 500);

    m_model = new BlockModel(0, this);
    m_block = new Block(m_model, this);
    setCentralWidget(m_block);

    connect(m_block, &Block::setpointChanged, this, &MainWindow::onSetpoint);
    connect(m_block, &Block::fireChannel, this, &MainWindow::onFireChannel);
    connect(m_block, &Block::requestResize, this, &MainWindow::onResizeRequest);
    connect(m_model, &BlockModel::pyroFired, this, &MainWindow::onPyroFired);

    // Панель управления
    QToolBar *toolbar = addToolBar("Управление");
    m_pingButton = new QPushButton("Тест связи", this);
    m_startStopButton = new QPushButton("Старт опроса", this);
    m_startStopButton->setCheckable(true);
    toolbar->addWidget(m_pingButton);
    toolbar->addWidget(m_startStopButton);
    connect(m_pingButton, &QPushButton::clicked, this, &MainWindow::onPingClicked);
    connect(m_startStopButton, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);

    // Таймер чтения данных из очереди (30 fps)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateFromMaster);
    m_updateTimer->start(33);
}

MainWindow::~MainWindow()
{
    stopMaster();   // гарантируем остановку мастера
    m_updateTimer->stop();
}

void MainWindow::updateFromMaster()
{
    bkd::core::TickData data;
    while (bkd::core::g_masterToGui.pop(data)) {
        m_model->updateFromTickData(data);
    }
}

void MainWindow::startMaster()
{
    if (m_master) return;   // уже запущен

    using namespace bkd::core;
    using namespace bkd::network;

    auto network = std::make_unique<RealYlsNetwork>(YLS_IP, YLS_PORT);
    if (!network->start()) {
        statusBar()->showMessage("Ошибка инициализации сети", 2000);
        return;
    }

    m_master = std::make_unique<Master>(std::move(network),
                                        g_guiToMaster,
                                        g_masterToGui,
                                        g_masterToLogger);
    m_master->start();
    statusBar()->showMessage("Тест запущен", 1000);
}

void MainWindow::stopMaster()
{
    if (!m_master) return;
    m_master->stop();
    m_master.reset();
    statusBar()->showMessage("Тест остановлен", 1000);
}

void MainWindow::onStartStopClicked()
{
    bool start = m_startStopButton->isChecked();
    if (start) {
        startMaster();
        m_startStopButton->setText("Стоп опроса");
    } else {
        stopMaster();
        m_startStopButton->setText("Старт опроса");
    }
}

void MainWindow::onSetpoint(int gaugeIndex, double value) {
    if (!m_master) return;

    int16_t val = static_cast<int16_t>(value);
    // Сохраняем setpoint в модели для отображения
    m_model->setSetpoint(gaugeIndex, val);

    // Отправляем команду в мастер
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_DRIVE_ANGLES;
    cmd.block = 0;
    int16_t cur1 = m_block->currentAngle1();   // текущий угол с ЯЛС
    int16_t cur2 = m_block->currentAngle2();
    if (gaugeIndex == 0) cur1 = val;
    else cur2 = val;
    cmd.drive.angles[0] = cur1;
    cmd.drive.angles[1] = cur2;
    cmd.drive.angles[2] = 0;
    cmd.drive.angles[3] = 0;
    bkd::core::g_guiToMaster.push(cmd);
}

void MainWindow::onFireChannel(int channel)
{
    if (!m_master) return;
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
    if (pingHost(bkd::core::YLS_IP, 2000))
        statusBar()->showMessage("ЯЛС доступен", 2000);
    else
        statusBar()->showMessage("ЯЛС НЕ ДОСТУПЕН", 2000);
}
