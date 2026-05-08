#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QVariantList>
#include "queues.h"
namespace bkd::core {
struct TickData;
struct GuiCommand;
template<typename T, size_t Capacity> class SPSCQueue;
}

class BlockModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int blockId READ blockId WRITE setBlockId NOTIFY blockIdChanged)
    Q_PROPERTY(int currentAngle1 READ currentAngle1 NOTIFY dataChanged)
    Q_PROPERTY(int currentAngle2 READ currentAngle2 NOTIFY dataChanged)
    Q_PROPERTY(int currentAngle3 READ currentAngle3 NOTIFY dataChanged)
    Q_PROPERTY(int currentAngle4 READ currentAngle4 NOTIFY dataChanged)
    Q_PROPERTY(int pyroMask READ pyroMask NOTIFY dataChanged)
    Q_PROPERTY(bool active READ active NOTIFY dataChanged)

public:
    explicit BlockModel(QObject *parent = nullptr);

    int blockId() const { return m_blockId; }
    void setBlockId(int id);

    int currentAngle1() const { return m_angles[0]; }
    int currentAngle2() const { return m_angles[1]; }
    int currentAngle3() const { return m_angles[2]; }
    int currentAngle4() const { return m_angles[3]; }
    int pyroMask() const { return m_pyroMask; }
    bool active() const { return m_active; }

    // Обновление из TickData (вызывается из потока GUI)
    void updateFromTickData(const bkd::core::TickData& data);

    // Команды, отправляемые в эмулятор
    Q_INVOKABLE void setTargetAngle(int angleIndex, int value);
    Q_INVOKABLE void firePyro(int channel);
    Q_INVOKABLE void clearHistory();

    // Для графика
    Q_INVOKABLE QVariantList angleHistory(int angleIndex) const;
    Q_INVOKABLE QVariantList commandHistory(int angleIndex) const;

signals:
    void blockIdChanged();
    void dataChanged(); // общий сигнал для всех свойств
    void historyChanged();

private:
    int m_blockId = 0; // 0..11
    int m_angles[4] = {0,0,0,0};
    int m_pyroMask = 0;
    bool m_active = true;

    struct Point { double time; int value; };
    QVector<Point> m_angleHistory[4];
    QVector<Point> m_commandHistory[4];

    void addAnglePoint(int index, int value);
    void addCommandPoint(int index, int value);
};

#endif // BLOCKMODEL_H
