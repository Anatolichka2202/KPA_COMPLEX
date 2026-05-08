#ifndef GAUGEWIDGET_H
#define GAUGEWIDGET_H

#include <QWidget>
#include <QColor>

class GaugeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double setpoint READ setpoint WRITE setSetpoint NOTIFY setpointChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setArcColor)
    Q_PROPERTY(QColor needleColor READ needleColor WRITE setNeedleColor)
    Q_PROPERTY(int arcThickness READ arcThickness WRITE setArcThickness)
    Q_PROPERTY(double needleLengthFactor READ needleLengthFactor WRITE setNeedleLengthFactor)
    Q_PROPERTY(bool setpointHandleVisible READ setpointHandleVisible WRITE setSetpointHandleVisible)
    Q_PROPERTY(double centerVerticalOffset READ centerVerticalOffset WRITE setCenterVerticalOffset)

public:
    explicit GaugeWidget(QWidget *parent = nullptr);
    double value() const { return m_value; }
    double minValue() const { return m_min; }
    double maxValue() const { return m_max; }
    QColor arcColor() const { return m_arcColor; }
    QColor needleColor() const { return m_needleColor; }
    double setpoint() const { return m_setpoint; }
    int arcThickness() const { return m_arcThickness; }
    double needleLengthFactor() const { return m_needleLengthFactor; }
    bool setpointHandleVisible() const { return m_showHandle; }
    double centerVerticalOffset() const { return m_centerVerticalOffset; }
    int cachedRadius() const { return m_radius; }
    int cachedCenterX() const { return m_centerX; }
    int cachedCenterY() const { return m_centerY; }

public slots:
    void setValue(double value);
    void setMinValue(double min);
    void setMaxValue(double max);
    void setSetpoint(double setpoint);
    void setArcColor(const QColor &color);
    void setNeedleColor(const QColor &color);
    void setArcThickness(int thickness);
    void setNeedleLengthFactor(double factor);
    void setSetpointHandleVisible(bool visible);
    void setCenterVerticalOffset(double offset);

signals:
    void valueChanged(double value);
    void setpointChanged(double setpoint);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    double m_value = 0.0;
    double m_min = -140.0;
    double m_max = 140.0;
    QColor m_arcColor = QColor(0, 100, 200);
    QColor m_needleColor = QColor(220, 0, 0);
    QColor m_backgroundColor = QColor(240, 240, 240);
    QColor m_textColor = Qt::black;

    int m_arcThickness = 12;
    double m_needleLengthFactor = 0.7;
    bool m_showHandle = true;
    double m_centerVerticalOffset = 0.0;

    int m_centerX = 0, m_centerY = 0;
    int m_radius = 0;
    double m_setpoint = 0.0;
    bool m_draggingSetpoint = false;

    void updateGeometryCache();
    double valueToAngle(double val) const;
    double angleToValue(int angleDeg) const;
    QPointF setpointHandlePosition() const;
    bool isNearHandle(const QPointF &pos) const;
};

#endif
