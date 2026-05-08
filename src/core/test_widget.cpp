#include "test_widget.h"
#include "types.h"
#include "lockfree_queues.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>
#include <QProcess>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <random>
#include <QDateTime>
TestWidget::TestWidget(GuiCmdQueue& from_gui, TickDataQueue& to_gui, QWidget* parent)
    : QWidget(parent), from_gui_(from_gui), to_gui_(to_gui)
{
    setWindowTitle("КПА - Стенд (блок 0)");
    resize(900, 700);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Верхняя панель: тест, старт/стоп, статус
    QHBoxLayout* topLayout = new QHBoxLayout;
    testButton = new QPushButton("Тест связи", this);
    startStopButton = new QPushButton("Старт опроса", this);
    startStopButton->setCheckable(true);
    statusLabel = new QLabel("Статус: не проверено", this);
    timeLabel = new QLabel("Время: --:--:---", this);  // добавить
    topLayout->addWidget(testButton);
    topLayout->addWidget(startStopButton);
    topLayout->addWidget(statusLabel);
    topLayout->addWidget(timeLabel);                   // добавить
    mainLayout->addLayout(topLayout);

    // Группа углов: поля ввода, отображение текущих, график
    QGroupBox* anglesGroup = new QGroupBox("Углы (блок 0)", this);
    QVBoxLayout* anglesGroupLayout = new QVBoxLayout(anglesGroup);
    QGridLayout* anglesLayout = new QGridLayout;
    for (int i = 0; i < 4; ++i) {
        QLabel* label = new QLabel(QString("Угол %1:").arg(i+1), this);
        angleEdits[i] = new QLineEdit(this);
        angleEdits[i]->setText("0");
        anglesDisplay[i] = new QLabel(i >= 2 ? "N/A" : "0", this);
        anglesLayout->addWidget(label, i, 0);
        anglesLayout->addWidget(angleEdits[i], i, 1);
        anglesLayout->addWidget(anglesDisplay[i], i, 2);
        if (i >= 2) {
            angleEdits[i]->setEnabled(false);
            angleEdits[i]->setText("N/A");
        }
    }
    setAnglesButton = new QPushButton("Установить углы", this);
    anglesLayout->addWidget(setAnglesButton, 4, 0, 1, 3);
    anglesGroupLayout->addLayout(anglesLayout);

    // В группе углов добавим слайдеры для углов 1 и 2
    for (int i = 0; i < 2; ++i) {
        angleSliders[i] = new QSlider(Qt::Horizontal, this);
        angleSliders[i]->setRange(0, 65535);
        angleSliders[i]->setValue(0);
        connect(angleSliders[i], &QSlider::valueChanged, this, [this, i](int val) {
            angleEdits[i]->setText(QString::number(val));
            // можно автоотправлять, но пока оставим кнопку
        });
        connect(angleEdits[i], &QLineEdit::textChanged, this, [this, i](const QString& text) {
            bool ok;
            int val = text.toInt(&ok);
            if (ok && val >= 0 && val <= 65535)
                angleSliders[i]->setValue(val);
        });
        // добавим слайдеры в layout
        anglesLayout->addWidget(angleSliders[i], i, 3);
    }
    // для углов 3 и 4 слайдеры не нужны, можно задизейблить


    // График для углов 1 и 2
    chartView = new QChartView(this);
    QChart* chart = new QChart();
    chart->setTitle("Текущие углы (1 и 2)");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    series[0] = new QLineSeries();
    series[0]->setName("Угол 1");
    series[1] = new QLineSeries();
    series[1]->setName("Угол 2");
    chart->addSeries(series[0]);
    chart->addSeries(series[1]);
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Время (кадры)");
    axisX->setRange(0, 100);
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Значение");
    axisY->setRange(0, 65535);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series[0]->attachAxis(axisX);
    series[0]->attachAxis(axisY);
    series[1]->attachAxis(axisX);
    series[1]->attachAxis(axisY);
    chartView->setChart(chart);
    chartView->setMinimumHeight(200);
    anglesGroupLayout->addWidget(chartView);
    mainLayout->addWidget(anglesGroup);

    // Группа пиро
    QGroupBox* pyroGroup = new QGroupBox("Пиросредства (блок 0)", this);
    QGridLayout* pyroLayout = new QGridLayout(pyroGroup);
    for (int i = 0; i < 8; ++i) {
        pyroButtons[i] = new QPushButton(QString("Пиро %1").arg(i+1), this);
        pyroButtons[i]->setCheckable(true);
        pyroLayout->addWidget(pyroButtons[i], i/4, i%4);
        connect(pyroButtons[i], &QPushButton::toggled, this, [this, i](bool checked) {
            onPyroToggled(i, checked);
        });
    }
    mainLayout->addWidget(pyroGroup);

    // Добавляем внизу главного лэйаута лог пиро
    QGroupBox* pyroLogGroup = new QGroupBox("Журнал пиросрабатываний", this);
    QVBoxLayout* logLayout = new QVBoxLayout(pyroLogGroup);
    pyroLog = new QTextEdit(this);
    pyroLog->setReadOnly(true);
    pyroLog->setMaximumHeight(150);
    clearLogButton = new QPushButton("Очистить журнал");
    connect(clearLogButton, &QPushButton::clicked, [this]() { pyroLog->clear(); });
    logLayout->addWidget(pyroLog);
    logLayout->addWidget(clearLogButton);
    mainLayout->addWidget(pyroLogGroup);

    connect(testButton, &QPushButton::clicked, this, &TestWidget::onTestButtonClicked);
    connect(startStopButton, &QPushButton::clicked, this, &TestWidget::onStartStopClicked);
    connect(setAnglesButton, &QPushButton::clicked, this, &TestWidget::onSetAnglesClicked);
}

TestWidget::~TestWidget() {}

void TestWidget::processIncoming() {
    bkd::core::TickData data;
    while (to_gui_.pop(data)) {
        if (data.response_received) {
            std::array<uint16_t,4> angles;
            for (int i = 0; i < 4; ++i)
                angles[i] = data.incoming.drives[0][i];
            lastAngles = angles;
            updateAnglesDisplay(angles);
            updateChart(angles, data.tick_time);
            uint8_t pyroMask = data.incoming.pyro_masks[0];
            // логируем изменения
            if (pyroMask != lastReceivedPyroMask) {
                for (int i = 0; i < 8; ++i) {
                    bool was = (lastReceivedPyroMask >> i) & 1;
                    bool now = (pyroMask >> i) & 1;
                    if (now != was) {
                        logPyroEvent(i, now, data.tick_time);
                    }
                }
                lastReceivedPyroMask = pyroMask;
            }
            updatePyroDisplay(lastSentPyroMask, pyroMask);
        }
        QString globalTime = QDateTime::fromMSecsSinceEpoch(data.tick_time / 1000).toString("hh:mm:ss.zzz");
        timeLabel->setText(globalTime);
    }

}

void TestWidget::updateAnglesDisplay(const std::array<uint16_t,4>& angles) {
    for (int i = 0; i < 2; ++i) {
        anglesDisplay[i]->setText(QString::number(angles[i]));
        if (angleSliders[i]->value() != angles[i]) {
            // обновляем слайдер, но без зацикливания сигналов
            angleSliders[i]->blockSignals(true);
            angleSliders[i]->setValue(angles[i]);
            angleSliders[i]->blockSignals(false);
        }
    }
    anglesDisplay[2]->setText("N/A");
    anglesDisplay[3]->setText("N/A");
}

void TestWidget::updateChart(const std::array<uint16_t,4>& angles, uint64_t tickTime) {
    // Если это первый вызов, запоминаем начальное время
    if (startTimestamp == 0) {
        startTimestamp = tickTime;
        lastTimestamp = tickTime;
    }
    // Преобразуем в секунды с плавающей точкой
    double timeSec = (tickTime - startTimestamp) / 1000000.0; // микросекунды -> секунды
    series[0]->append(timeSec, angles[0]);
    series[1]->append(timeSec, angles[1]);

    // Ограничиваем отображение последними 10 секундами
    if (series[0]->count() > 0) {
        double minTime = timeSec - 10.0;
        chartView->chart()->axisX()->setRange(minTime, timeSec);
    }
}

void TestWidget::updatePyroDisplay(uint8_t sentMask, uint8_t receivedMask) {
    for (int i = 0; i < 8; ++i) {
        bool sent = (sentMask >> i) & 1;
        bool received = (receivedMask >> i) & 1;
        QString style;
        if (sent && !received)
            style = "background-color: blue; color: white;";
        else if (received)
            style = "background-color: red; color: white;";
        else
            style = "background-color: gray; color: black;";
        pyroButtons[i]->setStyleSheet(style);
        // состояние checkable показываем отдельно, но не отключаем
        pyroButtons[i]->setChecked(sent);
    }
}

bool TestWidget::pingHost(const QString& host, int timeoutMs) {
#ifdef Q_OS_WIN
    QStringList args = { "-n", "1", "-w", QString::number(timeoutMs), host };
#else
    QStringList args = { "-c", "1", "-W", QString::number(timeoutMs/1000), host };
#endif
    QProcess ping;
    ping.start("ping", args);
    return ping.waitForFinished(timeoutMs + 1000) && ping.exitCode() == 0;
}

void TestWidget::onTestButtonClicked() {
    statusLabel->setText("Статус: проверка связи...");
    // реальный ping
    if (pingHost(bkd::core::YLS_IP, 2000)) {
        statusLabel->setText("Статус: ЯЛС доступен");
        // можно отправить тестовый пакет (опционально)
    } else {
        statusLabel->setText("Статус: ЯЛС НЕ ДОСТУПЕН");
        QMessageBox::warning(this, "Ошибка", "Нет ответа от ЯЛС");
    }
}

void TestWidget::onStartStopClicked() {
    pollingActive = startStopButton->isChecked();
    bkd::core::GuiCommand cmd;
    cmd.type = pollingActive ? bkd::core::GuiCommand::START_POLLING : bkd::core::GuiCommand::STOP_POLLING;
    cmd.block = currentBlock;
    from_gui_.push(cmd);
    startStopButton->setText(pollingActive ? "Стоп опроса" : "Старт опроса");
}

void TestWidget::onSetAnglesClicked() {
    std::array<uint16_t,4> angles = {0,0,0,0};
    bool ok = true;
    for (int i = 0; i < 2; ++i) {  // только углы 1 и 2
        angles[i] = angleEdits[i]->text().toUShort(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Ошибка", QString("Некорректное значение угла %1").arg(i+1));
            return;
        }
    }
    // углы 3 и 4 обнуляем (не используются)
    angles[2] = 0;
    angles[3] = 0;
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_DRIVE_ANGLES;
    cmd.block = currentBlock;
    for (int i = 0; i < 4; ++i)
        cmd.drive.angles[i] = angles[i];
    from_gui_.push(cmd);
}

void TestWidget::onPyroToggled(int idx, bool checked) {
    uint8_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        if (pyroButtons[i]->isChecked())
            mask |= (1 << i);
    }
    lastSentPyroMask = mask;
    bkd::core::GuiCommand cmd;
    cmd.type = bkd::core::GuiCommand::SET_PYRO_MASK;
    cmd.block = currentBlock;
    cmd.pyro_mask = mask;
    from_gui_.push(cmd);
}
void TestWidget::logPyroEvent(int pyroIndex, bool activated, uint64_t timestamp) {
    QString timeStr = QDateTime::fromMSecsSinceEpoch(timestamp / 1000).toString("hh:mm:ss.zzz");
    QString event = QString("[%1] Пиро %2 %3").arg(timeStr).arg(pyroIndex+1).arg(activated ? "СРАБОТАЛ" : "ОТКЛЮЧЁН");
    pyroLog->append(event);
}
