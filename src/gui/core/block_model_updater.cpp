#include "block_model_updater.h"
#include "blockmodel.h"
#include "types.h"

BlockModelUpdater::BlockModelUpdater(BlockModel* model, TickDataQueue& queue, QObject* parent)
    : QObject(parent), m_model(model), m_queue(queue) {
}

void BlockModelUpdater::start(int intervalMs) {
    m_timer.setInterval(intervalMs);
    connect(&m_timer, &QTimer::timeout, this, &BlockModelUpdater::update);
    m_timer.start();
}

void BlockModelUpdater::update() {
    bkd::core::TickData data;
    while (m_queue.pop(data)) {
        m_model->updateFromTickData(data);
    }
}
