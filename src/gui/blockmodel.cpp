#include "blockmodel.h"
#include "core/types.h"
#include <QDateTime>

BlockModel::BlockModel(int blockIndex, QObject *parent)
    : QObject(parent), m_blockIndex(blockIndex)
{
}

void BlockModel::updateFromTickData(const bkd::core::TickData &data)
{
    bool anglesChangedFlag = false;
    double newAngle1 = data.incoming.drives[m_blockIndex][0];
    double newAngle2 = data.incoming.drives[m_blockIndex][1];
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
        emit historyChanged();
    }

    uint8_t newMask = data.incoming.pyro_masks[m_blockIndex];
    if (m_pyroMask != newMask) {
        // Определяем, какие каналы только что сработали
        uint8_t changed = newMask ^ m_pyroMask;
        for (int i = 0; i < 8; ++i) {
            if (changed & (1 << i)) {
                // Если бит стал 1 -> сработало
                if (newMask & (1 << i)) {
                    qint64 now = QDateTime::currentMSecsSinceEpoch();
                    if (m_pendingFireTime.contains(i+1)) {
                        qint64 requestTime = m_pendingFireTime.take(i+1);
                        emit pyroFired(i+1, requestTime, now);
                    } else {
                        // Сработало без запроса? (например, из эмуляции)
                        emit pyroFired(i+1, 0, now);
                    }
                } else {
                    // Бит сброшен – такого быть не должно (пиро одноразовые), игнорируем
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
