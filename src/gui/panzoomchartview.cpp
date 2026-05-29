#include "panzoomchartview.h"
#include <QtCharts/QChart>
#include <QScrollBar>

PanZoomChartView::PanZoomChartView(QWidget *parent)
    : QChartView(parent)
{
    setRubberBand(QChartView::NoRubberBand); // отключаем стандартный резиновый зум
    setInteractive(true);                    // разрешаем взаимодействие
}

void PanZoomChartView::wheelEvent(QWheelEvent *event)
{
    QChart *chart = this->chart();
    if (!chart) return;

    QPointF cursorPos = event->position();
    QPointF scenePos = mapToScene(cursorPos.toPoint());
    QPointF chartPos = chart->mapFromScene(scenePos);

    qreal factor = (event->angleDelta().y() > 0) ? m_zoomFactor : 1.0 / m_zoomFactor;

    // Зум по оси X
    QRectF plotArea = chart->plotArea();
    qreal xCenter = chartPos.x();
    qreal newWidth = plotArea.width() / factor;

    // Ограничиваем минимальную и максимальную ширину
    if (newWidth < 0.1) newWidth = 0.1;
    if (newWidth > 1e6) newWidth = 1e6;

    qreal xLeft = xCenter - (xCenter - plotArea.left()) / factor;
    chart->zoomIn(QRectF(xLeft, plotArea.top(), newWidth, plotArea.height()));

    event->accept();
}

void PanZoomChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QChartView::mousePressEvent(event);
}

void PanZoomChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPointF delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        // Сдвигаем диаграмму на величину delta (в координатах графика)
        chart()->scroll(-delta.x(), 0); // только по X
        event->accept();
        return;
    }
    QChartView::mouseMoveEvent(event);
}

void PanZoomChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QChartView::mouseReleaseEvent(event);
}
