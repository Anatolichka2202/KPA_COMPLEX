#ifndef CORESTUB_H
#define CORESTUB_H

#include <QObject>
#include <QTimer>
#include <random>

class CoreStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool emulationMode READ emulationMode WRITE setEmulationMode NOTIFY emulationModeChanged)
    Q_PROPERTY(bool emulationRunning READ isEmulationRunning NOTIFY emulationRunningChanged)

public:
    explicit CoreStub(QObject *parent = nullptr);
    static CoreStub* instance();

    bool emulationMode() const;
    void setEmulationMode(bool enable);

    bool isEmulationRunning() const;

    // Управление эмуляцией
    Q_INVOKABLE void startEmulation();
    Q_INVOKABLE void stopEmulation();

    // Команды от UI
    Q_INVOKABLE void setTargetAngle(int blockId, int angleIndex, float value);
    Q_INVOKABLE void firePyro(int blockId, int channel);
    Q_INVOKABLE void requestBlockState(int blockId); // для принудительного обновления

signals:
    void angleChanged(int blockId, int angleIndex, float value);
    void pyroMaskChanged(int blockId, int mask);
    void blockActiveChanged(int blockId, bool active);
    void emulationModeChanged();
    void emulationRunningChanged();

private slots:
    void onTimer();

private:
    static CoreStub* m_instance;
    QTimer m_timer;
    bool m_emulationMode = true;   // true – эмуляция, false – реальная сеть (пока не реализовано)
    bool m_emulationRunning = false;

    struct BlockData {
        float targetAngles[4] = {0,0,0,0};
        float currentAngles[4] = {0,0,0,0};
        int pyroMask = 0;      // 0..255, бит 1..8
        bool active = true;
    };
    BlockData m_blocks[12];    // индексы 0..11 соответствуют блокам 1..12

    void simulateMovement();
    void updateAndEmitAngle(int blockIdx, int angleIndex);
};

#endif // CORESTUB_H
