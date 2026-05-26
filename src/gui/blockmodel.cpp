#include "blockmodel.h"
#include "core/types.h"
#include "core/queues.h"
#include <QDateTime>

BlockModel::BlockModel(int blockIndex, QObject *parent)
    : QObject(parent), m_blockIndex(blockIndex)
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
}

void BlockModel::updateFromTickData(const bkd::core::TickData &data)
{
    // Используем первый процессор (индекс 0)
    int16_t newAngle1 = data.incoming.data.ykp[m_blockIndex][0][0];
    int16_t newAngle2 = data.incoming.data.ykp[m_blockIndex][0][1];

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double timeSec = (now - m_startTime) / 1000.0;
    emit newDataPoint(timeSec, newAngle1, newAngle2);

    // Обновляем текущие углы (для шкалы), только если изменились
    if (m_angle1 != newAngle1 || m_angle2 != newAngle2) {
        m_angle1 = newAngle1;
        m_angle2 = newAngle2;
        emit anglesChanged(m_angle1, m_angle2);
    }

    // Пиро – как было, но с индексом процессора 0
    uint8_t newMask = data.incoming.data.yps_bkd[m_blockIndex][0];
    if (m_pyroMask != newMask) {
        uint8_t changed = newMask ^ m_pyroMask;
        for (int i = 0; i < 8; ++i) {
            if (changed & (1 << i)) {
                if (newMask & (1 << i)) {
                    qint64 nowFire = QDateTime::currentMSecsSinceEpoch();
                    if (m_pendingFireTime.contains(i+1)) {
                        qint64 requestTime = m_pendingFireTime.take(i+1);
                        emit pyroFired(i+1, requestTime, nowFire);
                    } else {
                        emit pyroFired(i+1, 0, nowFire);
                    }
                }
            }
        }
        m_pyroMask = newMask;
        emit pyroMaskChanged(m_pyroMask);
    }
}
void BlockModel::addHistoryPoint(QVector<QPair<qint64, double>> &hist, double val)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    hist.append({now, val});
    while (hist.size() > MAX_HISTORY)
        hist.removeFirst();
}

double BlockModel::rawToAngle(int16_t raw)
{

    return raw;

}

void BlockModel::onPyroRequested(int channel)
{
    // Запоминаем время запроса пользователя
    m_pendingFireTime[channel] = QDateTime::currentMSecsSinceEpoch();
    // Отправляем команду в Master
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_PYRO_MASK;
    cmd.block = m_blockIndex;
    uint8_t newMask = m_pyroMask | (1 << (channel-1));
    cmd.pyro_mask = newMask;
    bkd::core::g_guiToMaster.push(cmd);
}

void BlockModel::setSetpoint(int gaugeIndex, int16_t value) {
    if (gaugeIndex == 0) {
        if (m_setpoint1 == value) return;
        m_setpoint1 = value;
    } else {
        if (m_setpoint2 == value) return;
        m_setpoint2 = value;
    }
    emit setpointsChanged(m_setpoint1, m_setpoint2);
}
