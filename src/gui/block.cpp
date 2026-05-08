#include "block.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QLineEdit>

Block::Block(BlockModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    ui.setupUi(this);

    // Подключаем сигналы от AdvancedGauge
    connect(ui.gaude_inf_1, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint1);
    connect(ui.gaude_inf_2, &AdvancedGauge::setpointChanged, this, &Block::onSetpoint2);
    connect(ui.unwrap_btn, &QPushButton::clicked, this, &Block::onUnwrapClicked);

    // Поля ввода углов (добавьте два QLineEdit в .ui, назвав angle1_edit, angle2_edit)
    // Если их нет, создайте динамически:
    if (!ui.angle1_edit) {
        ui.angle1_edit = new QLineEdit(this);
        ui.angle2_edit = new QLineEdit(this);
        // разместите их в нужном layout, например, в горизонтальный layout
        QHBoxLayout *editLayout = new QHBoxLayout;
        editLayout->addWidget(new QLabel("Угол 1:"));
        editLayout->addWidget(ui.angle1_edit);
        editLayout->addWidget(new QLabel("Угол 2:"));
        editLayout->addWidget(ui.angle2_edit);
        ui.basic_layout->insertLayout(1, editLayout);
    }
    connect(ui.angle1_edit, &QLineEdit::editingFinished, this, &Block::onAngle1EditingFinished);
    connect(ui.angle2_edit, &QLineEdit::editingFinished, this, &Block::onAngle2EditingFinished);

    // Пиро – превратить QLabel в QPushButton
    for (int i = 1; i <= 8; ++i) {
        QPushButton *btn = findChild<QPushButton*>(QString("pyro_%1").arg(i));
        if (!btn) {
            // Если в .ui были QLabel, заменяем их динамически
            QLabel *lbl = findChild<QLabel*>(QString("pyro_%1").arg(i));
            if (lbl) {
                btn = new QPushButton(lbl->text(), this);
                btn->setObjectName(QString("pyro_%1").arg(i));
                btn->setCheckable(true);
                QLayout *layout = lbl->parentWidget()->layout();
                int index = layout->indexOf(lbl);
                delete lbl;
                layout->insertWidget(index, btn);
            }
        }
        if (btn) {
            connect(btn, &QPushButton::clicked, this, [this, i]() {
                onPyroClicked(i);
            });
        }
    }

    // Подключаем модель
    connect(m_model, &BlockModel::anglesChanged, this, &Block::updateAngles);
    connect(m_model, &BlockModel::pyroMaskChanged, this, &Block::updatePyroMask);
    connect(m_model, &BlockModel::newDataPoint, this, &Block::addDataPoint);
    connect(m_model, &BlockModel::pyroFired, this, [this](int ch, qint64 req, qint64 conf) {
        if (req == 0) return;
        double timeSec = (conf - m_model->startTime()) / 1000.0;
        // Маркер ставим на текущие углы (можно и на другие значения)
        addPyroMarker(timeSec, currentAngle1(), currentAngle2());
    });

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
    m_chartView->chart()->setUseOpenGL(true); // GPU ускорение

    QVBoxLayout *frameLayout = new QVBoxLayout(ui.frame_for_graphs);
    frameLayout->setContentsMargins(0,0,0,0);
    frameLayout->addWidget(m_chartView);
}

void Block::updateAngles(double angle1, double angle2)
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

void Block::addDataPoint(double timeSec, double angle1, double angle2)
{
    m_series1->append(timeSec, angle1);
    m_series2->append(timeSec, angle2);
    // Ограничиваем длину истории (200 точек)
    while (m_series1->count() > 200) {
        m_series1->remove(0);
        m_series2->remove(0);
    }
    // Сдвигаем ось X, чтобы показывать последние 10 секунд
    if (timeSec > 10) {
        m_chartView->chart()->axisX()->setRange(timeSec - 10, timeSec);
    }
}

void Block::addPyroMarker(double timeSec, double angle1, double angle2)
{
    m_pyroMarkers->append(timeSec, angle1);
    m_pyroMarkers->append(timeSec, angle2);
}

void Block::addSetpointMarker(double timeSec, double angle1, double angle2)
{
    m_setpointMarkers->append(timeSec, angle1);
    m_setpointMarkers->append(timeSec, angle2);
}

void Block::onSetpoint1(double sp)
{
    emit setpointChanged(0, sp);
    // Отметка на графике (установка уставки)
    double cur = currentAngle1();
    addSetpointMarker(QDateTime::currentMSecsSinceEpoch() / 1000.0, sp, currentAngle2());
}

void Block::onSetpoint2(double sp)
{
    emit setpointChanged(1, sp);
    addSetpointMarker(QDateTime::currentMSecsSinceEpoch() / 1000.0, currentAngle1(), sp);
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
    m_model->onPyroRequested(channel);
}
