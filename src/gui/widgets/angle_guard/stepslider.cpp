#include "stepslider.h"
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QStyle>
#include <QtMath>
#include <QDebug>

StepSlider::StepSlider(QWidget *parent)
    : QSlider(parent)
    , m_repeatTimer(new QTimer(this))
    , m_lastDelta(0.0)
    , m_repeatDelay(500)
    , m_repeatInterval(100)
{
    setOrientation(Qt::Horizontal);
    setMinimum(-3);
    setMaximum(3);
    setTickPosition(QSlider::TicksBelow);
    setTickInterval(1);
    setSingleStep(1);
    setPageStep(1);
    setValue(0);
    setTracking(false); // чтобы не было лишних сигналов при движении

    m_repeatTimer->setSingleShot(false);
    connect(m_repeatTimer, &QTimer::timeout, this, &StepSlider::onRepeatTimeout);
}

void StepSlider::setRepeatDelay(int ms) { m_repeatDelay = ms; }
void StepSlider::setRepeatInterval(int ms) { m_repeatInterval = ms; }

int StepSlider::stepFromPos(const QPoint &pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    int sliderMin = grooveRect.x();
    int sliderMax = grooveRect.x() + grooveRect.width();
    int x = pos.x();
    x = qBound(sliderMin, x, sliderMax);
    int range = maximum() - minimum();
    if (range <= 0) return 0;
    int step = minimum() + (x - sliderMin) * range / (sliderMax - sliderMin);
    step = qBound(minimum(), step, maximum());
    return step;
}

double StepSlider::deltaFromStep(int step) const
{
    switch (step) {
    case -3: return -1.5;
    case -2: return -1.0;
    case -1: return -0.5;
    case  1: return  0.5;
    case  2: return  1.0;
    case  3: return  1.5;
    default: return 0.0;
    }
}

void StepSlider::mousePressEvent(QMouseEvent *event)
{
    int step = stepFromPos(event->pos());
    double delta = deltaFromStep(step);
    qDebug() << "StepSlider: step" << step << "delta" << delta;
    if (!qFuzzyIsNull(delta)) {
        emit stepApplied(delta);
        startRepeat(delta);
    } else {
        stopRepeat();
    }
    QSlider::mousePressEvent(event); // сохраняем стандартную анимацию
}

void StepSlider::mouseReleaseEvent(QMouseEvent *event)
{
    stopRepeat();
    setValue(0);
    QSlider::mouseReleaseEvent(event);
}

void StepSlider::startRepeat(double delta)
{
    m_lastDelta = delta;
    m_repeatTimer->setInterval(m_repeatDelay);
    m_repeatTimer->start();
}

void StepSlider::stopRepeat()
{
    m_repeatTimer->stop();
}

void StepSlider::onRepeatTimeout()
{
    emit stepApplied(m_lastDelta);
    m_repeatTimer->setInterval(m_repeatInterval);
}
