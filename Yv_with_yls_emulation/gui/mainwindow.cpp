#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logic.h"
#include "configmanager.h"
#include "networkfactory.h"
#include "blockwidget.h"
#include "Protocol.h"

#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>
#include <array>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow constructor";
    ui->setupUi(this);

    // Настройка интерфейса
    setupUI();
    setupStatusBar();

    // Создаём блоки
    block_widgets_.resize(12);
    for (int i = 0; i < 12; ++i) {
        block_widgets_[i] = new BlockWidget(i, this);
        ui->gridLayout->addWidget(block_widgets_[i], i / 4, i % 4);

        connect(block_widgets_[i], &BlockWidget::pyroMaskChanged,
                this, &MainWindow::onPyroMaskChanged);
    }

    ui->scrollAreaWidgetContents->setMinimumHeight(1100);

    for (int row = 0; row < 3; ++row) {
        ui->gridLayout->setRowStretch(row, 1);
    }
    for (int col = 0; col < 4; ++col) {
        ui->gridLayout->setColumnStretch(col, 1);
    }

    // Загружаем конфигурацию
    YAV::Config config = YAV::ConfigManager::load("yv_config.ini");

    qDebug() << "=== CONFIG LOADED ===";
    qDebug() << "Config yls_mode value:" << static_cast<int>(config.yls_mode);

    switch (config.yls_mode) {
    case YAV::Config::YLSMode::FULL_EMULATE:
        qDebug() << "Mode: FULL_EMULATE";
        break;
    case YAV::Config::YLSMode::EMULATE_YLS:
        qDebug() << "Mode: EMULATE_YLS";
        break;
    case YAV::Config::YLSMode::REAL:
        qDebug() << "Mode: REAL";
        break;
    default:
        qDebug() << "Mode: UNKNOWN";
    }

    qDebug() << "Config file:" << "yv_config.ini";
    qDebug() << "==========================";


    std::unique_ptr<YAV::Network::IUdpClient> network_client;
    network_client = YAV::Network::createQtUdpClient(); // Всегда Qt

    // Создаём логику
    yv_logic_ = std::make_unique<YAV::YVLogic>(std::move(network_client));

    // Устанавливаем колбэки
    yv_logic_->setDataCallback([this](const BKD::Protocol::YLSToYVPacket& data) {
        QMetaObject::invokeMethod(this, [this, data]() {                        //потокобезопасный Инвок, для многопоточности
            onDataForGUI(data);
        });
    });

    yv_logic_->setErrorCallback([this](const std::string& error) {
        QMetaObject::invokeMethod(this, [this, error]() {
            onErrorOccurred(QString::fromStdString(error));
        });
    });

    yv_logic_->setStatusCallback([this](const std::string& status) {
        QMetaObject::invokeMethod(this, [this, status]() {
            onStatusChanged(QString::fromStdString(status));
        });
    });

    // Инициализируем логику
    if (!yv_logic_->initialize(config)) {
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось инициализировать логику ЯВ\nПроверьте конфигурацию");
        return;
    }

    // Настраиваем таймеры GUI
    setupTimers();


    status_label_->setText("Готов к работе");
    qDebug() << "MainWindow initialized";
};

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::onStopButtonClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);

    ui->stopButton->setEnabled(false);
}

void MainWindow::setupStatusBar()
{
    status_label_ = new QLabel("Инициализация...");
    mode_label_ = new QLabel("Режим: Эмуляция");
    stats_label_ = new QLabel("Получено: 0 пакетов");

    statusBar()->addWidget(status_label_);
    statusBar()->addWidget(mode_label_);
    statusBar()->addPermanentWidget(stats_label_);
}

void MainWindow::setupTimers()
{
    // Основной таймер для отправки пакетов (50 Гц = 20 мс)
    send_timer_.setInterval(20);
    connect(&send_timer_, &QTimer::timeout, this, [this]() {
        if (yv_logic_ && is_running_) {
            yv_logic_->sendCurrentPacket();
        }
    });

    // Таймер обновления GUI (10 Гц = 100 мс)
    gui_update_timer_.setInterval(100);
    connect(&gui_update_timer_, &QTimer::timeout, this, &MainWindow::updateGUIFromData);
    qDebug() << "Timers setup: send=" << send_timer_.interval()
             << "ms, gui=" << gui_update_timer_.interval() << "ms";
}


                                        // УПРАВЛЕНИЕ КНОПКАМИ

void MainWindow::onStartButtonClicked() {                                   //управление кнопкой старт
    if (!yv_logic_) {
        QMessageBox::warning(this, "Ошибка", "Логика не инициализирована");
        return;
    }

    qDebug() << "=== START BUTTON CLICKED ===";
    qDebug() << "Запуск YVLogic...";

    yv_logic_->start();
    is_running_ = true;

    // Запускаем таймеры
    send_timer_.start();
    gui_update_timer_.start();

    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->settingsButton->setEnabled(false);

    status_label_->setText("Работает (50 Гц)");
    qDebug() << "Таймеры запущены: отправка=" << send_timer_.interval()
             << "мс, GUI=" << gui_update_timer_.interval() << "мс";
}

void MainWindow::onStopButtonClicked()      //управление кнопкой стоп
{
    if (yv_logic_) {
        yv_logic_->stop();
        is_running_ = false;
    }

    // Останавливаем таймеры
    send_timer_.stop();
    gui_update_timer_.stop();

    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->settingsButton->setEnabled(true);

    status_label_->setText("Остановлен");
    qDebug() << "YVLogic stopped";
}

void MainWindow::onSettingsButtonClicked()  // управление кнопкой настройка
{
    QMessageBox::information(this, "Настройки",
                             "Настройки пока доступны только через файл yv_config.ini");
}

                                //



void MainWindow::onPyroMaskChanged(int blockIndex, quint8 mask) {
    if (!yv_logic_ || !is_running_) return;

    yv_logic_->setPyroMask(blockIndex, mask);
    qDebug() << "User changed pyro mask for block" << blockIndex
             << "to 0x" << QString::number(mask, 16);
}

void MainWindow::onDataForGUI(const BKD::Protocol::YLSToYVPacket& data)
{
    packets_received_++;
    stats_label_->setText(QString("Получено: %1 пакетов").arg(packets_received_));
}

void MainWindow::onErrorOccurred(const QString& error)                      //выводит ошибки, ок
{
    status_label_->setText("Ошибка: " + error);
    qWarning() << "YVLogic error:" << error;

    statusBar()->setStyleSheet("QStatusBar { background-color: #ffcccc; }");
    QTimer::singleShot(3000, [this]() {
        statusBar()->setStyleSheet("");
    });
}

void MainWindow::onStatusChanged(const QString& status)
{
    status_label_->setText(status);
    qDebug() << "Status:" << status;
}

void MainWindow::updateGUIFromData() {
    if (!yv_logic_ || !is_running_) return;

    BKD::Protocol::YLSToYVPacket data = yv_logic_->getCurrentData();
    uint16_t active_mask = data.getBlockMask();

    static int update_counter = 0;
    update_counter++;

    if (update_counter % 10 == 0) { // Выводим каждые 10 обновлений
        qDebug() << "GUI Update #" << update_counter << ", active mask: 0x"
                 << QString::number(active_mask, 16);
    }

    for (int block = 0; block < 12; ++block) {
        bool is_active = (active_mask & (1 << block)) != 0;
        block_widgets_[block]->setConnectionStatus(is_active, false);

        if (is_active) {
            QVector<double> angles = {
                static_cast<double>(data.drives[block][0]),
                static_cast<double>(data.drives[block][1]),
                static_cast<double>(data.drives[block][2])
            };
            uint8_t pyro_mask = data.pyro_masks[block];

            if (update_counter % 10 == 0) {
                qDebug() << "  Block" << block
                         << "angles:" << angles
                         << "pyro: 0x" << QString::number(pyro_mask, 16);
            }


            block_widgets_[block]->updateData(angles, pyro_mask);
        }
    }
    QApplication::processEvents();

}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Отложенное обновление для плавности
    QTimer::singleShot(10, this, [this]() {
        ui->scrollAreaWidgetContents->adjustSize();
        ui->gridLayout->update();
    });
}





