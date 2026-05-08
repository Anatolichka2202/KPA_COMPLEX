#ifndef BLOCKMODEL_UPDATER_H
#define BLOCKMODEL_UPDATER_H
#include "queues.h"
#include <QObject>
#include <QTimer>

namespace bkd::core {
template<typename T, size_t Cap> class SPSCQueue;
struct TickData;
}

class BlockModel;

class BlockModelUpdater : public QObject {
    Q_OBJECT
public:
    using TickDataQueue = bkd::core::SPSCQueue<bkd::core::TickData, 64>;

    BlockModelUpdater(BlockModel* model, TickDataQueue& queue, QObject* parent = nullptr);
    void start(int intervalMs = 30);

private slots:
    void update();

private:
    BlockModel* m_model;
    TickDataQueue& m_queue;
    QTimer m_timer;
};

#endif // BLOCKMODEL_UPDATER_H
