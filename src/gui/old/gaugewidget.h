#ifndef GAUGEWIDGET_H
#define GAUGEWIDGET_H

#include <QWidget>

class GaugeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor)
    Q_PROPERTY(QColor needleColor READ needleColor WRITE setNeedleColor)

public:
    explicit GaugeWidget(QWidget *parent = nullptr);

    int value() const { return m_value; }
    int minValue() const { return m_min; }
    int maxValue() const { return m_max; }
    QColor arcColor() const { return m_arcColor; }
    QColor needleColor() const { return m_needleColor; }

public slots:
    void setValue(int value);
    void setMinValue(int min);
    void setMaxValue(int max);
    void setArcColor(const QColor &color);
    void setNeedleColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override; // чтобы при изменении размера обновить координаты

private:
    int m_value = 0;
    int m_min = -140;
    int m_max = 140;
    QColor m_arcColor = QColor(0, 100, 200);   // синий
    QColor m_needleColor = QColor(220, 0, 0);   // красный
    QColor m_backgroundColor = QColor(240, 240, 240);
    QColor m_textColor = Qt::black;

    // Кэш для оптимальной отрисовки
    int m_side = 0;
    int m_centerX = 0, m_centerY = 0;
    int m_radius = 0;
    void updateGeometryCache();
    int valueToAngle(int val) const; // 0..180 градусов, где -140 -> 0, +140 -> 180
};

#endif // GAUGEWIDGET_H
