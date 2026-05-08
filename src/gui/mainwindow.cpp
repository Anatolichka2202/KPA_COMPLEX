#include "mainwindow.h"
#include <QPropertyAnimation>
#include <QDebug>
#include "core/queues.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("КПА - Тест блока");
    resize(600, 500);

    m_model = new BlockModel(0, this);
    m_block = new Block(m_model, this);
    setCentralWidget(m_block);

    connect(m_block, &Block::setpointChanged, this, &MainWindow::onSetpoint);
    connect(m_block, &Block::fireChannel, this, &MainWindow::onFireChannel);
    connect(m_block, &Block::requestResize, this, &MainWindow::onResizeRequest);
    connect(m_model, &BlockModel::pyroFired, this, &MainWindow::onPyroFired);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateFromEmulator);
    m_updateTimer->start(33);
}

MainWindow::~MainWindow()
{
    m_updateTimer->stop();
}

void MainWindow::updateFromEmulator()
{
    bkd::core::TickData data;
    while (bkd::core::g_emulatorToGui.pop(data)) {
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
    cmd.drive.angles[0] = static_cast<uint16_t>(cur1);
    cmd.drive.angles[1] = static_cast<uint16_t>(cur2);
    cmd.drive.angles[2] = 0;
    cmd.drive.angles[3] = 0;
    bkd::core::g_guiToEmulator.push(cmd);
}

void MainWindow::onFireChannel(int channel)
{
    // Запоминаем время запроса в модели
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    // Можно передать в модель, но модель уже содержит pendingFireTime
    // Мы просто отправим команду в эмулятор, а подтверждение придёт позже.
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_PYRO_MASK;
    cmd.block = 0;
    uint8_t newMask = m_block->currentPyroMask() | (1 << (channel-1));
    cmd.pyro_mask = newMask;
    bkd::core::g_guiToEmulator.push(cmd);

    // Также сообщаем модели, что запрос отправлен (для логирования)
    // Можно добавить метод в BlockModel, но пока оставим так:
    // Модель сама определит срабатывание по изменению маски.
}

void MainWindow::onPyroFired(int channel, qint64 requestTime, qint64 confirmTime)
{
    qint64 delay = (requestTime == 0) ? -1 : (confirmTime - requestTime);
    qDebug() << QString("Pyro %1 fired, delay = %2 ms").arg(channel).arg(delay);
    // Здесь можно добавить вывод в лог-виджет
}

void MainWindow::onResizeRequest(bool expand)
{
    animateResize(expand);
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
