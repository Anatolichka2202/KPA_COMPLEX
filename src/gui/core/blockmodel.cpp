#include "blockmodel.h"
#include "queues.h"
#include <QDateTime>

BlockModel::BlockModel(QObject *parent) : QObject(parent) {}

void BlockModel::setBlockId(int id) {
    if (m_blockId == id) return;
    m_blockId = id;
    emit blockIdChanged();
    // Очистка истории при смене блока – опционально
    clearHistory();
}

void BlockModel::updateFromTickData(const bkd::core::TickData& data) {
    // Предполагаем, что блок 0..11
    if (m_blockId < 0 || m_blockId >= 12) return;
    bool changed = false;
    for (int i = 0; i < 4; ++i) {
        int newVal = data.incoming.drives[m_blockId][i];
        if (m_angles[i] != newVal) {
            m_angles[i] = newVal;
            addAnglePoint(i, newVal);
            changed = true;
        }
    }
    int newMask = data.incoming.pyro_masks[m_blockId];
    if (m_pyroMask != newMask) {
        m_pyroMask = newMask;
        changed = true;
    }
    // active – не меняется, можно игнорировать
    if (changed)
        emit dataChanged();
}

void BlockModel::setTargetAngle(int angleIndex, int value) {
    if (angleIndex < 0 || angleIndex > 3) return;
    // Отправляем команду в эмулятор через глобальную очередь (см. main.cpp)
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_DRIVE_ANGLES;
    cmd.block = m_blockId;
    for (int i = 0; i < 4; ++i) cmd.drive.angles[i] = (i == angleIndex) ? value : m_angles[i];
    bkd::core::g_guiToEmulator.push(cmd);
    addCommandPoint(angleIndex, value);
    emit historyChanged();
}

void BlockModel::firePyro(int channel) {
    if (channel < 1 || channel > 8) return;
    uint8_t mask = 1 << (channel-1);
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_PYRO_MASK;
    cmd.block = m_blockId;
    cmd.pyro_mask = m_pyroMask | (1 << (channel-1));
    bkd::core::g_guiToEmulator.push(cmd);
}

void BlockModel::clearHistory() {
    for (int i = 0; i < 4; ++i) {
        m_angleHistory[i].clear();
        m_commandHistory[i].clear();
    }
    emit historyChanged();
}

void BlockModel::addAnglePoint(int index, int value) {
    double now = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    m_angleHistory[index].append({now, value});
    const int maxPoints = 2000;
    while (m_angleHistory[index].size() > maxPoints)
        m_angleHistory[index].removeFirst();
}

void BlockModel::addCommandPoint(int index, int value) {
    double now = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    m_commandHistory[index].append({now, value});
    const int maxCmd = 500;
    while (m_commandHistory[index].size() > maxCmd)
        m_commandHistory[index].removeFirst();
}

QVariantList BlockModel::angleHistory(int angleIndex) const {
    if (angleIndex < 0 || angleIndex > 3) return {};
    QVariantList res;
    for (const auto& pt : m_angleHistory[angleIndex]) {
        QVariantMap map;
        map["x"] = pt.time;
        map["y"] = pt.value;
        res.append(map);
    }
    return res;
}

QVariantList BlockModel::commandHistory(int angleIndex) const {
    if (angleIndex < 0 || angleIndex > 3) return {};
    QVariantList res;
    for (const auto& pt : m_commandHistory[angleIndex]) {
        QVariantMap map;
        map["x"] = pt.time;
        map["y"] = pt.value;
        res.append(map);
    }
    return res;
}
