#ifndef BLOCKWIDGET_H
#define BLOCKWIDGET_H

#include <QWidget>
#include <QtCharts>

namespace Ui {
class BlockWidget;
}

class BlockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlockWidget(int blockIndex, QWidget *parent = nullptr);
    ~BlockWidget();

    void updateData(const QVector<double> &driveAngles, quint8 pyroMask);
void setConnectionStatus(bool connected, bool timeout = false);
signals:
    void pyroMaskChanged(int blockIndex, quint8 mask);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupChart();
    void updateCircles(quint8 mask);
    void setupPyroClickHandlers();
    Ui::BlockWidget *ui;
    int block_index_;
    QBarSeries *series_;
    QBarSet *bar_set_;

    QVector<QWidget*> pyro_circles_;
    quint8 current_pyro_mask_ = 0;
};

#endif // BLOCKWIDGET_H
