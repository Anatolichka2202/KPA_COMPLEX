#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QSettings>
#include "advancedgauge.h"

class DebugPanel : public QWidget {
    Q_OBJECT
public:
    DebugPanel(GaugeWidget *gauge, QWidget *parent = nullptr) : QWidget(parent), m_gauge(gauge) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        // Отображение текущих параметров
        m_infoLabel = new QLabel;
        layout->addWidget(m_infoLabel);


        QPushButton *saveBtn = new QPushButton("Save Settings");
        QPushButton *loadBtn = new QPushButton("Load Settings");
        layout->addWidget(saveBtn);
        layout->addWidget(loadBtn);

        connect(saveBtn, &QPushButton::clicked, this, &DebugPanel::saveSettings);
        connect(loadBtn, &QPushButton::clicked, this, &DebugPanel::loadSettings);

        updateInfo();
        connect(m_gauge, &GaugeWidget::valueChanged, this, &DebugPanel::updateInfo);
        connect(m_gauge, &GaugeWidget::setpointChanged, this, &DebugPanel::updateInfo);
    }

    void updateInfo() {
        QString info = QString("Radius: %1  Center: (%2,%3)  Offset: %4\n")
        .arg(m_gauge->property("radius").toInt())
                .arg(m_gauge->property("centerX").toInt())
                .arg(m_gauge->property("centerY").toInt())
                .arg(m_gauge->centerVerticalOffset())
            + QString("Arc thickness: %1  Needle length factor: %2\n")
                  .arg(m_gauge->arcThickness())
                  .arg(m_gauge->needleLengthFactor());
        m_infoLabel->setText(info);
    }

private slots:
    void saveSettings() {
        QSettings settings("MyCompany", "GaugeDemo");
        settings.setValue("arcThickness", m_gauge->arcThickness());
        settings.setValue("needleLengthFactor", m_gauge->needleLengthFactor());
        settings.setValue("arcColor", m_gauge->arcColor());
        settings.setValue("needleColor", m_gauge->needleColor());
        settings.setValue("centerVerticalOffset", m_gauge->centerVerticalOffset());
        settings.setValue("setpointHandleVisible", m_gauge->setpointHandleVisible());
    }

    void loadSettings() {
        QSettings settings("MyCompany", "GaugeDemo");
        m_gauge->setArcThickness(settings.value("arcThickness", 12).toInt());
        m_gauge->setNeedleLengthFactor(settings.value("needleLengthFactor", 0.7).toDouble());
        m_gauge->setArcColor(settings.value("arcColor", QColor(0,100,200)).value<QColor>());
        m_gauge->setNeedleColor(settings.value("needleColor", QColor(220,0,0)).value<QColor>());
        m_gauge->setCenterVerticalOffset(settings.value("centerVerticalOffset", 0.0).toDouble());
        m_gauge->setSetpointHandleVisible(settings.value("setpointHandleVisible", true).toBool());
        updateInfo();
    }

private:
    GaugeWidget *m_gauge;
    QLabel *m_infoLabel;
};

class DemoWindow : public QMainWindow
{
public:
    DemoWindow() {
        setWindowTitle("Gauge Widget Live Editor");
        QWidget *central = new QWidget;
        setCentralWidget(central);
        QVBoxLayout *mainLayout = new QVBoxLayout(central);

        m_gauge = new AdvancedGauge;
        m_gauge->setMinimumHeight(300);
        mainLayout->addWidget(m_gauge, 2);

        // Панель управления
        QWidget *panel = new QWidget;
        QVBoxLayout *panelLayout = new QVBoxLayout(panel);
        mainLayout->addWidget(panel, 1);

        // 1. Толщина дуги
        QHBoxLayout *thickLayout = new QHBoxLayout;
        thickLayout->addWidget(new QLabel("Arc thickness:"));
        QSlider *thickSlider = new QSlider(Qt::Horizontal);
        thickSlider->setRange(2, 30);
        thickSlider->setValue(m_gauge->findChild<GaugeWidget*>()->arcThickness());
        thickLayout->addWidget(thickSlider);
        panelLayout->addLayout(thickLayout);
        connect(thickSlider, &QSlider::valueChanged, [this](int val){
            m_gauge->findChild<GaugeWidget*>()->setArcThickness(val);
        });

        // 2. Длина стрелки
        QHBoxLayout *needleLenLayout = new QHBoxLayout;
        needleLenLayout->addWidget(new QLabel("Needle length:"));
        QSlider *lenSlider = new QSlider(Qt::Horizontal);
        lenSlider->setRange(30, 90);
        lenSlider->setValue(70);
        needleLenLayout->addWidget(lenSlider);
        panelLayout->addLayout(needleLenLayout);
        connect(lenSlider, &QSlider::valueChanged, [this](int val){
            m_gauge->findChild<GaugeWidget*>()->setNeedleLengthFactor(val / 100.0);
        });

        // 3. Цвет дуги
        QPushButton *arcColorBtn = new QPushButton("Change arc color");
        panelLayout->addWidget(arcColorBtn);
        connect(arcColorBtn, &QPushButton::clicked, [this](){
            QColor col = QColorDialog::getColor(m_gauge->findChild<GaugeWidget*>()->arcColor(), this);
            if (col.isValid())
                m_gauge->findChild<GaugeWidget*>()->setArcColor(col);
        });

        // 4. Цвет стрелки
        QPushButton *needleColorBtn = new QPushButton("Change needle color");
        panelLayout->addWidget(needleColorBtn);
        connect(needleColorBtn, &QPushButton::clicked, [this](){
            QColor col = QColorDialog::getColor(m_gauge->findChild<GaugeWidget*>()->needleColor(), this);
            if (col.isValid())
                m_gauge->findChild<GaugeWidget*>()->setNeedleColor(col);
        });

        // 5. Смещение центра дуги по вертикали
        QHBoxLayout *offsetLayout = new QHBoxLayout;
        offsetLayout->addWidget(new QLabel("Center Y offset:"));
        QSlider *offsetSlider = new QSlider(Qt::Horizontal);
        offsetSlider->setRange(-30, 30);
        offsetSlider->setValue(0);
        offsetLayout->addWidget(offsetSlider);
        panelLayout->addLayout(offsetLayout);
        connect(offsetSlider, &QSlider::valueChanged, [this](int val){
            m_gauge->findChild<GaugeWidget*>()->setCenterVerticalOffset(val);
        });

        // 6. Показать/скрыть ручку
        QPushButton *toggleHandleBtn = new QPushButton("Toggle setpoint handle");
        panelLayout->addWidget(toggleHandleBtn);
        connect(toggleHandleBtn, &QPushButton::clicked, [this](){
            GaugeWidget *gw = m_gauge->findChild<GaugeWidget*>();
            gw->setSetpointHandleVisible(!gw->setpointHandleVisible());
        });
    }

private:
    AdvancedGauge *m_gauge;
    DebugPanel *db;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DemoWindow w;
    w.resize(500, 600);
    w.show();
    return a.exec();
}
