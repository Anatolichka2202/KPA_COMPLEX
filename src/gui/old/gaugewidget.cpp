#include "gaugewidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QDebug>

GaugeWidget::GaugeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(100, 100); // чтобы хоть что-то было видно
}

void GaugeWidget::setValue(int value)
{
    if (value < m_min) value = m_min;
    if (value > m_max) value = m_max;
    if (m_value == value) return;
    m_value = value;
    update();
}

void GaugeWidget::setMinValue(int min)
{
    if (m_min == min) return;
    m_min = min;
    if (m_value < m_min) setValue(m_min);
    update();
}

void GaugeWidget::setMaxValue(int max)
{
    if (m_max == max) return;
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

void GaugeWidget::updateGeometryCache()
{
    int w = width();
    int h = height();
    m_side = qMin(w, h);
    m_centerX = w / 2;
    m_centerY = h - h/4;   // центрируем чуть выше низа, как на картинке (полукруг сверху)
    // Но можно по-другому: пусть дуга рисуется от центра, но обычно для спидометра центр внизу
    // Традиционно: полукруг, центр внизу по центру, радиус = min(w, h)/2 * 0.8
    m_radius = m_side * 0.4;
    // Дополнительно можно скорректировать позицию центра:
    // сделаем так: центр по X = w/2, по Y = h - 15 (как в старом QML коде)
    m_centerY = height() - 15;
}

int GaugeWidget::valueToAngle(int val) const
{
    // Преобразуем из диапазона [min..max] в угол [0..180] градусов
    // 0 градусов (левая точка), 90 (вершина), 180 (правая точка)
    int range = m_max - m_min;
    if (range == 0) return 90;
    double fraction = static_cast<double>(val - m_min) / range;
    return static_cast<int>(fraction * 180);
}

void GaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    updateGeometryCache();

    // Фон (прозрачный? можно закрасить)
    painter.fillRect(rect(), m_backgroundColor);

    // Координаты центра и радиус
    int cx = m_centerX;
    int cy = m_centerY;
    int radius = m_radius;

    // Рисуем серую фоновую дугу (от 0 до 180 градусов)
    QPen penArc;
    penArc.setWidth(12);
    penArc.setCapStyle(Qt::FlatCap);
    penArc.setColor(Qt::lightGray);
    painter.setPen(penArc);

    // drawArc(x, y, w, h, startAngle, spanAngle) - углы в 1/16 градуса
    // Начинаем с левой точки (180 градусов) и идём до правой (0) против часовой.
    // Для полукруга сверху: startAngle = 0 (правая точка), spanAngle = -180 (движение против часовой).
    // Удобнее: startAngle = 180 * 16, spanAngle = -180 * 16
    painter.drawArc(cx - radius, cy - radius, radius*2, radius*2, 180*16, -180*16);

    // Цветная дуга (текущее значение)
    int angle = valueToAngle(m_value);
    penArc.setColor(m_arcColor);
    painter.setPen(penArc);
    painter.drawArc(cx - radius, cy - radius, radius*2, radius*2, 180*16, -angle*16);

    // Рисуем стрелку
    double angleRad = angle * M_PI / 180.0;
    double needleLength = radius * 0.7;
    double endX = cx - needleLength * std::cos(angleRad);
    double endY = cy - needleLength * std::sin(angleRad);
    QPen needlePen(m_needleColor);
    needlePen.setWidth(3);
    painter.setPen(needlePen);
    painter.drawLine(cx, cy, endX, endY);

    // Рисуем кружок в центре
    painter.setBrush(m_needleColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 6, cy - 6, 12, 12);

    // Рисуем риски (не обязательно, но для красоты)
    painter.setPen(QPen(Qt::darkGray, 1));
    for (int v = m_min; v <= m_max; v += 20) {
        int a = valueToAngle(v);
        double rad = a * M_PI / 180.0;
        double innerLen = radius * 0.85;
        double outerLen = radius;
        double x1 = cx - innerLen * std::cos(rad);
        double y1 = cy - innerLen * std::sin(rad);
        double x2 = cx - outerLen * std::cos(rad);
        double y2 = cy - outerLen * std::sin(rad);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }
    // Подписи значений (опционально)
    painter.setPen(m_textColor);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    for (int v = m_min; v <= m_max; v += 20) {
        int a = valueToAngle(v);
        double rad = a * M_PI / 180.0;
        double labelRad = radius * 0.95;
        double x = cx - labelRad * std::cos(rad);
        double y = cy - labelRad * std::sin(rad) - 2;
        painter.drawText(QPointF(x, y), QString::number(v));
    }

    // Текстовое значение в центре внизу
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(cx - 25, cy + 25, 50, 25, Qt::AlignCenter, QString::number(m_value) + "°");
}

void GaugeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGeometryCache();
}
