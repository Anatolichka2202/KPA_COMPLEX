#ifndef PANZOOMCHARTVIEW_H
#define PANZOOMCHARTVIEW_H

#include <QChartView>
#include <QPointF>
#include <QWheelEvent>
#include <QMouseEvent>

class PanZoomChartView : public QChartView
{
    Q_OBJECT
public:
    explicit PanZoomChartView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_isPanning = false;
    QPointF m_lastMousePos;
    qreal m_zoomFactor = 1.2;
};

#endif // PANZOOMCHARTVIEW_H
