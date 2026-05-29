#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>
#include "blockmodel.h"
#include "block.h"
#include "core/queues.h"

#include "network/i_proxy_backend.h"
#include "network/proxy_core.h"
#include "network/windivert_proxy_backend.h"

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
    void onProxyClicked();
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
    QPushButton *m_proxyButton;
    std::unique_ptr<bkd::core::Master> m_master;

     QLabel *m_timeLabel;

    std::unique_ptr<bkd::proxy::IProxyBackend> m_proxyBackend;
    bool m_proxyModeActive = false;   // признак, что сейчас работает прокси
};

#endif // MAINWINDOW_H
