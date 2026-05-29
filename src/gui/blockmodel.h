#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QHash>
#include <QPointF>

namespace bkd::core {
struct TickData;
}

class BlockModel : public QObject
{
    Q_OBJECT
public:
    explicit BlockModel(int blockIndex, QObject *parent = nullptr);
    void updateFromTickData(const bkd::core::TickData &data);

    double currentAngle1() const { return m_angle1; }
    double currentAngle2() const { return m_angle2; }
    qint64 startTime() const { return m_startTime; }
    uint8_t pyroMask() const { return m_pyroMask; }

    // График обновляется инкрементально
    // Оставлены для совместимости, но можно удалить
    const QVector<QPair<qint64, double>> &history1() const { return m_history1; }
    const QVector<QPair<qint64, double>> &history2() const { return m_history2; }

    void setSetpoint(int gaugeIndex, int16_t value);
    int16_t setpoint1() const { return m_setpoint1; }
    int16_t setpoint2() const { return m_setpoint2; }

signals:
    void anglesChanged(int16_t angle1, int16_t angle2);
    void newDataPoint(double timeSec, int16_t angle1, int16_t angle2);
    void pyroMaskChanged(uint8_t mask);
    void pyroFired(int channel, qint64 requestTime, qint64 confirmTime);
    void setpointsChanged(int16_t sp1, int16_t sp2);
    void newDataPointsBatch(const QList <QPointF>& points1, const QList <QPointF>& points2);

public slots:
    void onPyroRequested(int channel); // вызывается GUI при клике на пиро

private:
    int m_blockIndex;
    double m_angle1 = 0.0, m_angle2 = 0.0;
    uint8_t m_pyroMask = 0;
    qint64 m_startTime = 0;
    uint64_t m_startTick = 0;

    int16_t m_setpoint1 = 0;
    int16_t m_setpoint2 = 0;

    // История для графика
    QVector<QPair<qint64, double>> m_history1;
    QVector<QPair<qint64, double>> m_history2;

    QHash<int, qint64> m_pendingFireTime; // channel -> request timestamp (мс)

    void addHistoryPoint(QVector<QPair<qint64, double>> &hist, double val);
    static double rawToAngle(int16_t raw);
    static constexpr int MAX_HISTORY = 200;

    QList<QPointF> m_buffer1;
    QList<QPointF> m_buffer2;

    uint64_t m_lastFlushTick = 0;
    static constexpr int BUFFER_FLUSH_COUNT = 5; // сбрасываем бач каждые 5 точек
    static constexpr uint64_t BUFFER_FLUSH_US = 50000; // или кажды n мс
};

#endif // BLOCKMODEL_H
