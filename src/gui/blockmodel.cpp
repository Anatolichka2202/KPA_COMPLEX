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
    // Преобразование кодов в градусы
    double newAngle1 = rawToAngle(data.incoming.drives[m_blockIndex][0]);
    double newAngle2 = rawToAngle(data.incoming.drives[m_blockIndex][1]);

    bool anglesChangedFlag = false;
    if (!qFuzzyCompare(m_angle1, newAngle1)) {
        m_angle1 = newAngle1;
        addHistoryPoint(m_history1, m_angle1);
        anglesChangedFlag = true;
    }
    if (!qFuzzyCompare(m_angle2, newAngle2)) {
        m_angle2 = newAngle2;
        addHistoryPoint(m_history2, m_angle2);
        anglesChangedFlag = true;
    }
    if (anglesChangedFlag) {
        emit anglesChanged(m_angle1, m_angle2);
        // Инкрементальное обновление графика
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        double timeSec = (now - m_startTime) / 1000.0;
        emit newDataPoint(timeSec, m_angle1, m_angle2);
    }

    uint8_t newMask = data.incoming.pyro_masks[m_blockIndex];
    if (m_pyroMask != newMask) {
        uint8_t changed = newMask ^ m_pyroMask;
        for (int i = 0; i < 8; ++i) {
            if (changed & (1 << i)) {
                if (newMask & (1 << i)) {
                    qint64 now = QDateTime::currentMSecsSinceEpoch();
                    if (m_pendingFireTime.contains(i+1)) {
                        qint64 requestTime = m_pendingFireTime.take(i+1);
                        emit pyroFired(i+1, requestTime, now);
                    } else {
                        emit pyroFired(i+1, 0, now);
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
    // raw в диапазоне -32768..32767, нужно преобразовать в -140..140 градусов
    // Подразумевается, что 0 соответствует 0°, а максимум (32767) – 140°
    // Формула: angle = (raw / 32767.0) * 140.0
    return (static_cast<double>(raw) / 32767.0) * 140.0;
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
