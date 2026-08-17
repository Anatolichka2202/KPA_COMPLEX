#include "advancedgauge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AdvancedGauge::AdvancedGauge(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_gauge = new GaugeWidget(this);
    m_slider = new StepSlider(this);
    layout->addWidget(m_gauge);
    layout->addWidget(m_slider);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    m_setpointLabel = new QLabel(this);
    m_currentLabel = new QLabel(this);
    labelLayout->addWidget(m_setpointLabel);
    labelLayout->addStretch();
    labelLayout->addWidget(m_currentLabel);
    layout->addLayout(labelLayout);

    connect(m_gauge, &GaugeWidget::valueChanged, this, [this](double v){
        m_currentLabel->setText(QString::number(v, 'f', 1) + "°");
    });
    connect(m_gauge, &GaugeWidget::setpointChanged, this, [this](double sp){
        m_setpointLabel->setText("Set: " + QString::number(sp, 'f', 1) + "°");
    });
    connect(m_slider, &StepSlider::stepApplied, this, &AdvancedGauge::onStepApplied);

    m_gauge->setValue(0.0);
    m_gauge->setSetpoint(0.0);
    m_gauge->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gauge->setMinimumHeight(200);
    layout->setStretchFactor(m_gauge, 3);
    layout->setStretchFactor(m_slider, 1);
}

double AdvancedGauge::value() const
{
    return m_gauge->value();
}

double AdvancedGauge::setpoint() const
{
    return m_gauge->setpoint();
}

void AdvancedGauge::setValue(double v)
{
    m_gauge->setValue(v);
}

void AdvancedGauge::setSetpoint(double sp)
{
    m_gauge->setSetpoint(sp);
}

void AdvancedGauge::onStepApplied(double delta)
{
    qDebug() << "StepSlider signal received, delta =" << delta;
    double newSetpoint = m_gauge->setpoint() + delta;
    qDebug() << "New setpoint =" << newSetpoint;
    m_gauge->setSetpoint(newSetpoint);
}
