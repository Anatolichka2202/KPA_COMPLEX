#include "blockwidget.h"
#include "ui_blockwidget.h"
#include <QChart>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QDebug>

BlockWidget::BlockWidget(int blockIndex, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BlockWidget)
    , block_index_(blockIndex)
    , current_pyro_mask_(0)
{
    qDebug() << "Creating BlockWidget" << blockIndex;
    ui->setupUi(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->blockLabel->setText(QString("БЛОК %1").arg(block_index_ + 1));
    setupChart();
    setupPyroClickHandlers();
     updateCircles(0);
    qDebug() << "BlockWidget" << blockIndex << "created";
}

BlockWidget::~BlockWidget()
{
    delete ui;
}

void BlockWidget::setupPyroClickHandlers()
{
    pyro_circles_ = {
        ui->pyro1, ui->pyro2, ui->pyro3, ui->pyro4,
        ui->pyro5, ui->pyro6, ui->pyro7, ui->pyro8
    };

    for (int i = 0; i < pyro_circles_.size(); ++i) {
        pyro_circles_[i]->installEventFilter(this);
    }
}

bool BlockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        for (int i = 0; i < pyro_circles_.size(); ++i) {
            if (obj == pyro_circles_[i]) {
                current_pyro_mask_ ^= (1 << i);
                updateCircles(current_pyro_mask_);
                emit pyroMaskChanged(block_index_, current_pyro_mask_);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void BlockWidget::setupChart()
{
    QChart *chart = new QChart();
    series_ = new QBarSeries();
    bar_set_ = new QBarSet("Углы приводов");

    *bar_set_ << 0 << 0 << 0;
    bar_set_->setColor(QColor(70, 130, 180));

    series_->append(bar_set_);
    chart->addSeries(series_);
    chart->setTitle(QString("Блок %1").arg(block_index_ + 1));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundRoundness(0);
    chart->setMargins(QMargins(2, 2, 2, 2));
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setTitleBrush(QBrush(Qt::black));
    chart->setTitleFont(QFont("Arial", 10, QFont::Bold));

    QStringList categories {"Пр.1", "Пр.2", "Пр.3"};
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsFont(QFont("Arial", 9, QFont::Bold));
    axisX->setTitleText("Приводы");
    axisX->setTitleFont(QFont("Arial", 9));
    chart->addAxis(axisX, Qt::AlignBottom);
    series_->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    // Начальный диапазон, будет автоматически подстраиваться
    axisY->setRange(-100, 100);
    axisY->setTitleText("Значение угла");
    axisY->setTitleFont(QFont("Arial", 9));
    axisY->setLabelsFont(QFont("Arial", 8));
    axisY->setLabelFormat("%.0f");
    chart->addAxis(axisY, Qt::AlignLeft);
    series_->attachAxis(axisY);

    chart->legend()->setVisible(false);
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartView->setBackgroundBrush(QBrush(Qt::lightGray));

    // Включаем масштабирование и перемещение графика
    ui->chartView->setRubberBand(QChartView::RectangleRubberBand);

}

void BlockWidget::updateData(const QVector<double> &driveAngles, quint8 pyroMask)
{
    if (!driveAngles.isEmpty()) {
        // Обновляем данные столбцов
        for (int i = 0; i < 3 && i < driveAngles.size(); ++i) {
            bar_set_->replace(i, driveAngles[i]);
        }

        // Автоматическое масштабирование оси Y
        QValueAxis *axisY = qobject_cast<QValueAxis*>(ui->chartView->chart()->axes(Qt::Vertical).first());
        if (axisY) {
            double minVal = driveAngles[0];
            double maxVal = driveAngles[0];

            // Находим min/max среди всех углов
            for (int i = 1; i < driveAngles.size(); ++i) {
                if (driveAngles[i] < minVal) minVal = driveAngles[i];
                if (driveAngles[i] > maxVal) maxVal = driveAngles[i];
            }

            // Добавляем 10% запаса с каждой стороны
            double range = maxVal - minVal;
            double margin = range * 0.1;

            if (range < 1.0) { // Если значения слишком близки
                minVal -= 1.0;
                maxVal += 1.0;
            } else {
                minVal -= margin;
                maxVal += margin;
            }

            axisY->setRange(minVal, maxVal);
        }

        // Обновляем маску пиро
        current_pyro_mask_ = pyroMask;
        updateCircles(pyroMask);

        // Форсируем обновление
        ui->chartView->chart()->update();
        ui->chartView->repaint();
    }

    qDebug() << "Block" << block_index_ << "updated. Angles:" << driveAngles
             << "Pyro mask: 0x" << QString::number(pyroMask, 16);
}

void BlockWidget::updateCircles(quint8 mask)
{
    for (int i = 0; i < pyro_circles_.size() && i < 8; ++i) {
        bool isActive = (mask >> i) & 1;
        QString color = isActive ? "#FF4444" : "#CCCCCC";
        QString borderColor = isActive ? "#AA0000" : "#666666";
        pyro_circles_[i]->setStyleSheet(
            QString("background-color: %1; border: 2px solid %2; border-radius: 10px;")
                .arg(color).arg(borderColor)
            );
    }
}

void BlockWidget::setConnectionStatus(bool connected, bool timeout ) {
    QString border_color = "#555555"; // Серый по умолчанию

    if (connected) {
        border_color = "#00AA00"; // Зелёный - подключен
    } else if (timeout) {
        border_color = "#FFAA00"; // Оранжевый - таймаут
    } else {
        border_color = "#AA0000"; // Красный - ошибка
    }

    setStyleSheet(QString("QWidget#BlockWidget { border: 2px solid %1; }").arg(border_color));
}
