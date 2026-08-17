#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <memory>

namespace BKD {
namespace Protocol {
struct YLSToYVPacket;
}
}

namespace YAV {
class YVLogic;
class Config;
}

class BlockWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onSettingsButtonClicked();
    void onPyroMaskChanged(int blockIndex, quint8 mask);

    // Коллбэки из YVLogic
    void onDataForGUI(const BKD::Protocol::YLSToYVPacket& data);
    void onErrorOccurred(const QString& error);
    void onStatusChanged(const QString& status);



private:
    void setupUI();
    void setupStatusBar();
    void setupTimers();
    void updateGUIFromData();


    Ui::MainWindow *ui;
    std::unique_ptr<YAV::YVLogic> yv_logic_;

    QVector<BlockWidget*> block_widgets_;

    QLabel *status_label_;
    QLabel *mode_label_;
    QLabel *stats_label_;

    // ТАЙМЕРЫ ТОЛЬКО В GUI
    QTimer send_timer_;          // Отправка пакетов (50 Гц)
    QTimer gui_update_timer_;    // Обновление GUI (10 Гц)

    uint64_t packets_received_ = 0;
    bool is_running_ = false;
protected:
    void resizeEvent(QResizeEvent *event) override;

};

#endif // MAINWINDOW_H
