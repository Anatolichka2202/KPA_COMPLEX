#include "block.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDateTime>
#include <QPushButton>

Block::Block(BlockModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    ui.setupUi(this);

    connect(ui.gaude_inf_1, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint1);
    connect(ui.gaude_inf_2, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint2);
    connect(ui.unwrap_btn, &QPushButton::clicked, this, &Block::onUnwrapClicked);

    // Поля ввода углов уже есть в .ui, просто подключаем
    connect(ui.angle1_edit, &QLineEdit::editingFinished, this, &Block::onAngle1EditingFinished);
    connect(ui.angle2_edit, &QLineEdit::editingFinished, this, &Block::onAngle2EditingFinished);

    // Подключаем кнопки пиро (в .ui они уже QPushButton)
    for (int i = 1; i <= 8; ++i) {
        QPushButton *btn = findChild<QPushButton*>(QString("pyro_%1").arg(i));
        if (btn) {
            connect(btn, &QPushButton::clicked, this, [this, i]() { onPyroClicked(i); });
        }
    }

    // Подключаем модель
    connect(m_model, &BlockModel::anglesChanged, this, &Block::updateAngles);
    connect(m_model, &BlockModel::pyroMaskChanged, this, &Block::updatePyroMask);
    connect(m_model, &BlockModel::newDataPoint, this, &Block::addDataPoint);
    connect(m_model, &BlockModel::pyroFired, this, [this](int ch, qint64 req, qint64 conf) {
        if (req == 0) return;
        double timeSec = (conf - m_model->startTime()) / 1000.0;
        addPyroMarker(timeSec, currentAngle1(), currentAngle2());
    });
    connect(m_model, &BlockModel::setpointsChanged, this, &Block::onSetpointsChanged);

    setupChart();
    ui.frame_for_graphs->setVisible(false);
}

void Block::setupChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Углы во времени");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    m_series1 = new QLineSeries();
    m_series1->setName("Угол 1");
    m_series2 = new QLineSeries();
    m_series2->setName("Угол 2");
    m_pyroMarkers = new QScatterSeries();
    m_pyroMarkers->setName("Срабатывания пиро");
    m_pyroMarkers->setMarkerSize(8);
    m_pyroMarkers->setColor(Qt::red);
    m_setpointMarkers = new QScatterSeries();
    m_setpointMarkers->setName("Установки углов");
    m_setpointMarkers->setMarkerSize(6);
    m_setpointMarkers->setColor(Qt::blue);

    chart->addSeries(m_series1);
    chart->addSeries(m_series2);
    chart->addSeries(m_pyroMarkers);
    chart->addSeries(m_setpointMarkers);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Время, с");
    axisX->setRange(0, 10);
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(-140, 140);
    axisY->setTitleText("Градусы");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    m_series1->attachAxis(axisX);
    m_series1->attachAxis(axisY);
    m_series2->attachAxis(axisX);
    m_series2->attachAxis(axisY);
    m_pyroMarkers->attachAxis(axisX);
    m_pyroMarkers->attachAxis(axisY);
    m_setpointMarkers->attachAxis(axisX);
    m_setpointMarkers->attachAxis(axisY);

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    // QChart::setUseOpenGL отсутствует в Qt 6.2, закомментировано
    // m_chartView->chart()->setUseOpenGL(true);

    QVBoxLayout *frameLayout = new QVBoxLayout(ui.frame_for_graphs);
    frameLayout->setContentsMargins(0,0,0,0);
    frameLayout->addWidget(m_chartView);
}

void Block::updateAngles(int16_t angle1, int16_t angle2)
{
    ui.gaude_inf_1->setValue(angle1);
    ui.gaude_inf_2->setValue(angle2);
    ui.angle1_edit->setText(QString::number(angle1, 'f', 2));
    ui.angle2_edit->setText(QString::number(angle2, 'f', 2));
}

void Block::updatePyroMask(uint8_t mask)
{
    m_currentPyroMask = mask;
    for (int i = 1; i <= 8; ++i) {
        QPushButton *btn = findChild<QPushButton*>(QString("pyro_%1").arg(i));
        if (btn) {
            bool fired = (mask >> (i-1)) & 1;
            btn->setChecked(fired);
            btn->setStyleSheet(fired ?
                                   "background-color: red; border-radius: 5px; color: white;" :
                                   "background-color: lightgray; border-radius: 5px; color: black;");
        }
    }
}

void Block::addDataPoint(double timeSec, int16_t angle1, int16_t angle2)
{
    m_series1->append(timeSec, angle1);
    m_series2->append(timeSec, angle2);
    while (m_series1->count() > 200) {
        m_series1->remove(0);
        m_series2->remove(0);
    }
    if (timeSec > 10) {
        m_chartView->chart()->axisX()->setRange(timeSec - 10, timeSec);
    }
}

void Block::addPyroMarker(double timeSec, int16_t angle1, int16_t angle2)
{
    m_pyroMarkers->append(timeSec, angle1);
    m_pyroMarkers->append(timeSec, angle2);
}

void Block::addSetpointMarker(double timeSec, int16_t angle1, int16_t angle2)
{
    m_setpointMarkers->append(timeSec, angle1);
    m_setpointMarkers->append(timeSec, angle2);
}

void Block::onSetpoint1(int16_t sp) {
    emit setpointChanged(0, sp);
    double timeSec = (QDateTime::currentMSecsSinceEpoch() - m_model->startTime()) / 1000.0;
    // Добавляем точку только для угла 1, угол 2 – текущее значение с ЯЛС (не setpoint)
    addSetpointMarker(timeSec, sp, currentAngle2());
}

void Block::onSetpoint2(int16_t sp) {
    emit setpointChanged(1, sp);
    double timeSec = (QDateTime::currentMSecsSinceEpoch() - m_model->startTime()) / 1000.0;
    addSetpointMarker(timeSec, currentAngle1(), sp);
}

void Block::onUnwrapClicked()
{
    bool visible = ui.frame_for_graphs->isVisible();
    ui.frame_for_graphs->setVisible(!visible);
    emit requestResize(!visible);
}

void Block::onAngle1EditingFinished()
{
    bool ok;
    double val = ui.angle1_edit->text().toDouble(&ok);
    if (ok) {
        ui.gaude_inf_1->setSetpoint(val);
        emit setpointChanged(0, val);
    } else {
        ui.angle1_edit->setText(QString::number(ui.gaude_inf_1->value(), 'f', 2));
    }
}

void Block::onAngle2EditingFinished()
{
    bool ok;
    double val = ui.angle2_edit->text().toDouble(&ok);
    if (ok) {
        ui.gaude_inf_2->setSetpoint(val);
        emit setpointChanged(1, val);
    } else {
        ui.angle2_edit->setText(QString::number(ui.gaude_inf_2->value(), 'f', 2));
    }
}

void Block::onPyroClicked(int channel)
{
    emit fireChannel(channel);
}
void Block::onSetpointsChanged(int16_t sp1, int16_t sp2) {
    ui.angle1_edit->setText(QString::number(sp1));
    ui.angle2_edit->setText(QString::number(sp2));
}
