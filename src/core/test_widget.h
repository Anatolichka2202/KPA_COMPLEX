#ifndef CORE_TEST_WIDGET_H
#define CORE_TEST_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <array>
#include <QSlider>
#include <QTextEdit>
namespace bkd::core {
struct TickData;
struct GuiCommand;
template<typename T, size_t Capacity> class SPSCQueue;
}

//QT_CHARTS_USE_NAMESPACE

    class TestWidget : public QWidget {
    Q_OBJECT
public:
    using GuiCmdQueue = bkd::core::SPSCQueue<bkd::core::GuiCommand, 32>;
    using TickDataQueue = bkd::core::SPSCQueue<bkd::core::TickData, 64>;

    TestWidget(GuiCmdQueue& from_gui, TickDataQueue& to_gui, QWidget* parent = nullptr);
    ~TestWidget();

    void processIncoming(); // вызывать по таймеру

private slots:
    void onTestButtonClicked();
    void onStartStopClicked();
    void onSetAnglesClicked();
    void onPyroToggled(int idx, bool checked);

private:
    void updateAnglesDisplay(const std::array<uint16_t,4>& angles);
    void updatePyroDisplay(uint8_t sentMask, uint8_t receivedMask);
    void updateChart(const std::array<uint16_t,4>& angles, uint64_t tickTime);
    bool pingHost(const QString& host, int timeoutMs = 1000); // проверка доступности

    GuiCmdQueue& from_gui_;
    TickDataQueue& to_gui_;
    int currentBlock = 0;
    bool pollingActive = false;
    uint8_t lastSentPyroMask = 0;
    std::array<uint16_t,4> lastAngles;

    QPushButton* testButton;
    QLabel* statusLabel;
    QPushButton* startStopButton;
    QLineEdit* angleEdits[4];
    QPushButton* setAnglesButton;
    QPushButton* pyroButtons[8];
    QLabel* anglesDisplay[4];
    QChartView* chartView;
    QLineSeries* series[2]; // для углов 1 и 2 (можно добавить больше)
    int chartPoints = 0;
    QSlider* angleSliders[4];
    QLabel* timeLabel;              // для отображения глобального времени
    QLabel* pyroTimeLabel;          // для отображения времени последнего срабатывания пиро
    QPushButton* clearLogButton;    // очистить лог пиро
    QTextEdit* pyroLog;             // журнал срабатываний пиро

    uint64_t lastTimestamp = 0;     // для вычисления приращения на графике
    uint64_t startTimestamp = 0;    // начальное время для графика

    void updatePyroLog(const QString& event);
    uint8_t lastReceivedPyroMask = 0;
    void logPyroEvent(int pyroIndex, bool activated, uint64_t timestamp);
};

#endif // CORE_TEST_WIDGET_H
