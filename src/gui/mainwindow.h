#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "blockmodel.h"
#include "block.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFromEmulator();
    void onSetpoint(int gaugeIndex, double value);
    void onFireChannel(int channel);
    void onResizeRequest(bool expand);
    void onPyroFired(int channel, qint64 requestTime, qint64 confirmTime);

private:
    void animateResize(bool expand);

    BlockModel *m_model;
    Block *m_block;
    QTimer *m_updateTimer;
};

#endif // MAINWINDOW_H
