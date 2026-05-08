#ifndef BLOCK_H
#define BLOCK_H

#include <QWidget>
#include "ui_block.h"
#include "blockmodel.h"
#include <QChartView>
class Block : public QWidget
{
    Q_OBJECT
public:
    explicit Block(BlockModel *model, QWidget *parent = nullptr);

    // Методы для обновления из модели (вызываются по сигналам модели)
    void updateAngles(double angle1, double angle2);
    void updatePyroMask(uint8_t mask);
    void updateHistory();

    double currentAngle1() const { return ui.gaude_inf_1->value(); }
    double currentAngle2() const { return ui.gaude_inf_2->value(); }
    uint8_t currentPyroMask() const { return m_currentPyroMask; }

signals:
    void setpointChanged(int gaugeIndex, double value);
    void fireChannel(int channel);
    void requestResize(bool expand);

private slots:
    void onSetpoint1(double sp);
    void onSetpoint2(double sp);
    void onUnwrapClicked();

private:
    void setupChart();

    Ui::Block ui;
    BlockModel *m_model;
    uint8_t m_currentPyroMask = 0;
    QChartView *m_chartView = nullptr;
};

#endif // BLOCK_H
