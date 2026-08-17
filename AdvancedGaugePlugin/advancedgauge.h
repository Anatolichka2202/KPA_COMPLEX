#ifndef ADVANCEDGAUGE_H
#define ADVANCEDGAUGE_H

#include <QWidget>
#include <QLabel>
#include "gaugewidget.h"
#include "stepslider.h"

class AdvancedGauge : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double setpoint READ setpoint WRITE setSetpoint)
public:
    explicit AdvancedGauge(QWidget *parent = nullptr);
    double value() const;
    double setpoint() const;
public slots:
    void setValue(double v);
    void setSetpoint(double sp);
private slots:
    void onStepApplied(double delta);
private:
    GaugeWidget *m_gauge;
    StepSlider *m_slider;
    QLabel *m_currentLabel;
    QLabel *m_setpointLabel;
};
#endif
