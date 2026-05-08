#include "block.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

//using namespace QtCharts;

Block::Block(BlockModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    ui.setupUi(this);

    connect(ui.gaude_inf_1, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint1);
    connect(ui.gaude_inf_2, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint2);
    connect(ui.unwrap_btn, &QPushButton::clicked, this, &Block::onUnwrapClicked);

    // Подключаем сигналы модели к слотам виджета
    connect(m_model, &BlockModel::anglesChanged, this, &Block::updateAngles);
    connect(m_model, &BlockModel::pyroMaskChanged, this, &Block::updatePyroMask);
    connect(m_model, &BlockModel::historyChanged, this, &Block::updateHistory);

    setupChart();
    ui.frame_for_graphs->setVisible(false);
}

void Block::setupChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Углы во времени");
    QLineSeries *series1 = new QLineSeries();
    series1->setName("Угол 1");
    QLineSeries *series2 = new QLineSeries();
    series2->setName("Угол 2");
    chart->addSeries(series1);
    chart->addSeries(series2);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Время, с");
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(-150, 150);
    axisY->setTitleText("Градусы");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series1->attachAxis(axisX);
    series1->attachAxis(axisY);
    series2->attachAxis(axisX);
    series2->attachAxis(axisY);

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *frameLayout = new QVBoxLayout(ui.frame_for_graphs);
    frameLayout->setContentsMargins(0,0,0,0);
    frameLayout->addWidget(m_chartView);
}

void Block::updateAngles(double angle1, double angle2)
{
    ui.gaude_inf_1->setValue(angle1);
    ui.gaude_inf_2->setValue(angle2);
}

void Block::updatePyroMask(uint8_t mask)
{
    m_currentPyroMask = mask;
    QLabel *pyroLabels[8] = {ui.pyro_1, ui.pyro_2, ui.pyro_3, ui.pyro_4,
                             ui.pyro_5, ui.pyro_6, ui.pyro_7, ui.pyro_8};
    for (int i = 0; i < 8; ++i) {
        bool fired = (mask >> i) & 1;
        pyroLabels[i]->setStyleSheet(fired ?
                                         "background-color: red; border-radius: 5px;" :
                                         "background-color: lightgray; border-radius: 5px;");
    }
}

void Block::updateHistory()
{
    if (!m_chartView) return;
    QLineSeries *series1 = qobject_cast<QLineSeries*>(m_chartView->chart()->series()[0]);
    QLineSeries *series2 = qobject_cast<QLineSeries*>(m_chartView->chart()->series()[1]);
    series1->clear();
    series2->clear();

    const auto &hist1 = m_model->history1();
    const auto &hist2 = m_model->history2();
    // Для оси X используем секунды от первого отсчёта
    qint64 firstTime = hist1.isEmpty() ? 0 : hist1.first().first;
    for (const auto &pt : hist1) {
        double sec = (pt.first - firstTime) / 1000.0;
        series1->append(sec, pt.second);
    }
    for (const auto &pt : hist2) {
        double sec = (pt.first - firstTime) / 1000.0;
        series2->append(sec, pt.second);
    }
}

void Block::onSetpoint1(double sp)
{
    emit setpointChanged(0, sp);
}

void Block::onSetpoint2(double sp)
{
    emit setpointChanged(1, sp);
}

void Block::onUnwrapClicked()
{
    bool visible = ui.frame_for_graphs->isVisible();
    ui.frame_for_graphs->setVisible(!visible);
    emit requestResize(!visible);
}
