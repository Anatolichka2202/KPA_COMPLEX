#include "gaugewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QDebug>

GaugeWidget::GaugeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(120, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

// ---- Сеттеры ----
void GaugeWidget::setValue(double value)
{
    if (value < m_min) value = m_min;
    if (value > m_max) value = m_max;
    if (qFuzzyCompare(m_value, value)) return;
    m_value = value;
    update();
    emit valueChanged(m_value);
}

void GaugeWidget::setMinValue(double min)
{
    if (qFuzzyCompare(m_min, min)) return;
    m_min = min;
    if (m_value < m_min) setValue(m_min);
    update();
}

void GaugeWidget::setMaxValue(double max)
{
    if (qFuzzyCompare(m_max, max)) return;
    m_max = max;
    if (m_value > m_max) setValue(m_max);
    update();
}

void GaugeWidget::setArcColor(const QColor &color)
{
    m_arcColor = color;
    update();
}

void GaugeWidget::setNeedleColor(const QColor &color)
{
    m_needleColor = color;
    update();
}

void GaugeWidget::setSetpoint(double setpoint)
{
    qDebug() << "GaugeWidget::setSetpoint called with" << setpoint;
    if (setpoint < m_min) setpoint = m_min;
    if (setpoint > m_max) setpoint = m_max;
    if (qFuzzyCompare(m_setpoint, setpoint)) return;
    m_setpoint = setpoint;
    update();
    emit setpointChanged(m_setpoint);
}

void GaugeWidget::setArcThickness(int thickness)
{
    if (m_arcThickness == thickness) return;
    m_arcThickness = thickness;
    update();
}

void GaugeWidget::setNeedleLengthFactor(double factor)
{
    if (qFuzzyCompare(m_needleLengthFactor, factor)) return;
    m_needleLengthFactor = factor;
    update();
}

void GaugeWidget::setSetpointHandleVisible(bool visible)
{
    if (m_showHandle == visible) return;
    m_showHandle = visible;
    update();
}

void GaugeWidget::setCenterVerticalOffset(double offset)
{
    if (qFuzzyCompare(m_centerVerticalOffset, offset)) return;
    m_centerVerticalOffset = offset;
    updateGeometryCache();
    update();
}

// ---- Геометрия ----
void GaugeWidget::updateGeometryCache()
{
    int w = width();
    int h = height();
    m_centerX = w / 2;
    m_radius = qMin(w, h) * 0.4;
    // Центр по Y: низ виджета минус 35% радиуса + смещение
    m_centerY = h - static_cast<int>(m_radius * 0.35) + static_cast<int>(m_centerVerticalOffset);
    // Ограничения, чтобы дуга не уходила за края
    if (m_centerY - m_radius < 0) m_centerY = m_radius;
    if (m_centerY + m_radius > h) m_centerY = h - m_radius;
}

// ---- Преобразования углов ----
double GaugeWidget::valueToAngle(double val) const
{
    double range = m_max - m_min;
    if (qFuzzyIsNull(range)) return 90.0;
    double fraction = (val - m_min) / range;
    return fraction * 180.0;
}

double GaugeWidget::angleToValue(int angleDeg) const
{
    double fraction = static_cast<double>(angleDeg) / 180.0;
    return m_min + fraction * (m_max - m_min);
}

// ---- Позиция ручки setpoint ----
QPointF GaugeWidget::setpointHandlePosition() const
{
    double angle = valueToAngle(m_setpoint);
    double rad = angle * M_PI / 180.0;
    // Ручка расположена на дуге (радиус = m_radius)
    double x = m_centerX - m_radius * std::cos(rad);
    double y = m_centerY - m_radius * std::sin(rad);
    return QPointF(x, y);
}

bool GaugeWidget::isNearHandle(const QPointF &pos) const
{
    if (!m_showHandle) return false;
    QPointF handle = setpointHandlePosition();
    double dx = pos.x() - handle.x();
    double dy = pos.y() - handle.y();
    return std::hypot(dx, dy) <= 12.0;
}

// ---- Отрисовка ----
void GaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    updateGeometryCache();

    painter.fillRect(rect(), m_backgroundColor);
    int cx = m_centerX, cy = m_centerY, radius = m_radius;

    // 1. Фоновая дуга (вся от 0° до 180°, серая)
    QPen penArc;
    penArc.setWidth(m_arcThickness);
    penArc.setCapStyle(Qt::FlatCap);
    penArc.setColor(Qt::lightGray);
    painter.setPen(penArc);
    painter.drawArc(cx - radius, cy - radius, radius*2, radius*2, 180*16, -180*16);

    // 2. Цветная дуга (текущее значение) – рисуем от верхней точки (90°) до угла значения
    double angle = valueToAngle(m_value);      // угол значения в градусах (0..180)
    int startAngle16, spanAngle16;

    if (angle >= 90.0) {
        // Положительное значение – дуга вправо от 90° до angle
        startAngle16 = 90 * 16;                // 90° в 1/16 градуса
        spanAngle16 = static_cast<int>((angle - 90.0) * 16);
    } else {
        // Отрицательное значение – дуга влево от angle до 90°
        startAngle16 = static_cast<int>(angle * 16);
        spanAngle16 = static_cast<int>((90.0 - angle) * 16);
    }

    penArc.setColor(m_arcColor);
    painter.setPen(penArc);
    painter.drawArc(cx - radius, cy - radius, radius*2, radius*2,
                    startAngle16, spanAngle16);

    // 3. Стрелка текущего значения (без изменений)
    double angleRad = angle * M_PI / 180.0;
    double needleLength = radius * m_needleLengthFactor;
    double endX = cx - needleLength * std::cos(angleRad);
    double endY = cy - needleLength * std::sin(angleRad);
    QPen needlePen(m_needleColor, 3);
    painter.setPen(needlePen);
    painter.drawLine(cx, cy, endX, endY);

    // 4. Пунктирная линия для setpoint (аналогично, но рисуем от центра к углу setpoint)
    if (m_showHandle) {
        double setAngle = valueToAngle(m_setpoint);
        double setRad = setAngle * M_PI / 180.0;
        double setEndX = cx - needleLength * std::cos(setRad);
        double setEndY = cy - needleLength * std::sin(setRad);
        QPen setPen(m_needleColor, 2, Qt::DashLine);
        painter.setPen(setPen);
        painter.drawLine(cx, cy, setEndX, setEndY);
    }

    // 5. Ручка setpoint (жёлтый кружок) – остаётся на дуге
    if (m_showHandle) {
        QPointF handlePos = setpointHandlePosition();
        painter.setBrush(QColor(255, 200, 0));
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(handlePos, 8, 8);
    }

    // 6. Центральный кружок
    painter.setBrush(m_needleColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 6, cy - 6, 12, 12);

    // 7. Риски и подписи (без изменений)
    painter.setPen(QPen(Qt::darkGray, 1));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    for (double v = m_min; v <= m_max + 0.5; v += 20.0) {
        double a = valueToAngle(v);
        double rad = a * M_PI / 180.0;
        double innerLen = radius + 2;
        double outerLen = radius + 10;
        double x1 = cx - innerLen * std::cos(rad);
        double y1 = cy - innerLen * std::sin(rad);
        double x2 = cx - outerLen * std::cos(rad);
        double y2 = cy - outerLen * std::sin(rad);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));

        double labelRad = radius + 15;
        double x = cx - labelRad * std::cos(rad);
        double y = cy - labelRad * std::sin(rad) - 4;
        painter.drawText(QPointF(x, y), QString::number(static_cast<int>(v)));
    }

    // 8. Текущее и установленное значения (углы) на виджете
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.setPen(m_textColor);
    painter.drawText(10, height() - 10, QString("Cur: %1°").arg(m_value, 0, 'f', 1));
    painter.drawText(width() - 80, height() - 10, QString("Set: %1°").arg(m_setpoint, 0, 'f', 1));
}

// ---- Обработка мыши для перетаскивания ручки setpoint ----
void GaugeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isNearHandle(event->pos())) {
        m_draggingSetpoint = true;
    }
}

void GaugeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_draggingSetpoint) return;

    double dx = event->pos().x() - m_centerX;
    double dy = event->pos().y() - m_centerY;
    double angleRad = std::atan2(-dy, -dx);
    int angleDeg = static_cast<int>(std::round(angleRad * 180.0 / M_PI));
    if (angleDeg < 0) angleDeg += 360;
    angleDeg = qBound(0, angleDeg, 180);
    double newSetpoint = angleToValue(angleDeg);
    setSetpoint(newSetpoint);
}

void GaugeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_draggingSetpoint = false;
}

void GaugeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGeometryCache();
}
