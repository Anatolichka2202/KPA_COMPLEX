#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>
#include "blockmodel.h"
#include "block.h"
#include "core/queues.h"

// Forward declarations
namespace bkd::core { class Master; }
namespace bkd::network { class RealYlsNetwork; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateFromMaster();

private slots:
    void onSetpoint(int gaugeIndex, double value);
    void onFireChannel(int channel);
    void onResizeRequest(bool expand);
    void onPyroFired(int channel, qint64 requestTime, qint64 confirmTime);
    void onPingClicked();
    void onStartStopClicked();

private:
    void animateResize(bool expand);
    bool pingHost(const QString &host, int timeoutMs);
    void startMaster();
    void stopMaster();

    BlockModel *m_model;
    Block *m_block;
    QTimer *m_updateTimer;
    QPushButton *m_pingButton;
    QPushButton *m_startStopButton;
    std::unique_ptr<bkd::core::Master> m_master;
};

#endif // MAINWINDOW_H
