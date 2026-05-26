#ifndef BLOCK_H
#define BLOCK_H

#include <QWidget>
#include "ui_block.h"
#include "blockmodel.h"
#include <QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>

class Block : public QWidget
{
    Q_OBJECT
public:
    explicit Block(BlockModel *model, QWidget *parent = nullptr);

    void updateAngles(int16_t angle1, int16_t angle2);
    void updatePyroMask(uint8_t mask);
    void addDataPoint(double timeSec, int16_t angle1, int16_t angle2);
    void addPyroMarker(double timeSec, int16_t angle1, int16_t angle2);
    void addSetpointMarker(double timeSec, int16_t angle1, int16_t angle2);

    double currentAngle1() const { return ui.gaude_inf_1->value(); }
    double currentAngle2() const { return ui.gaude_inf_2->value(); }
    uint8_t currentPyroMask() const { return m_currentPyroMask; }

signals:
    void setpointChanged(int gaugeIndex, double value);
    void fireChannel(int channel);
    void requestResize(bool expand);

private slots:
    void onSetpoint1(int16_t sp);
    void onSetpoint2(int16_t sp);
    void onUnwrapClicked();
    void onAngle1EditingFinished();
    void onAngle2EditingFinished();
    void onPyroClicked(int channel);
    void onSetpointsChanged(int16_t sp1, int16_t sp2);

private:
    void setupChart();

    Ui::Block ui;
    BlockModel *m_model;
    uint8_t m_currentPyroMask = 0;
    QChartView *m_chartView = nullptr;
    QLineSeries *m_series1 = nullptr;
    QLineSeries *m_series2 = nullptr;
    QScatterSeries *m_pyroMarkers = nullptr;
    QScatterSeries *m_setpointMarkers = nullptr;
};

#endif // BLOCK_H
