#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QHash>

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
    uint8_t pyroMask() const { return m_pyroMask; }

    // История углов (для графика) – теперь не нужна, график обновляется инкрементально
    // Оставлены для совместимости, но можно удалить
    const QVector<QPair<qint64, double>> &history1() const { return m_history1; }
    const QVector<QPair<qint64, double>> &history2() const { return m_history2; }

signals:
    void anglesChanged(double angle1, double angle2);
    void pyroMaskChanged(uint8_t mask);
    void newDataPoint(double timeSec, double angle1, double angle2);
    void pyroFired(int channel, qint64 requestTime, qint64 confirmTime);

public slots:
    void onPyroRequested(int channel); // вызывается GUI при клике на пиро

private:
    int m_blockIndex;
    double m_angle1 = 0.0, m_angle2 = 0.0;
    uint8_t m_pyroMask = 0;
    qint64 m_startTime = 0;

    // История для графика (оставлена для обратной совместимости, но не обязательна)
    QVector<QPair<qint64, double>> m_history1;
    QVector<QPair<qint64, double>> m_history2;

    QHash<int, qint64> m_pendingFireTime; // channel -> request timestamp (мс)

    void addHistoryPoint(QVector<QPair<qint64, double>> &hist, double val);
    static double rawToAngle(int16_t raw);
    static constexpr int MAX_HISTORY = 200;
};

#endif // BLOCKMODEL_H
