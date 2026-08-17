#include "qtudpclient.h"
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QElapsedTimer>
#include <QDebug>

namespace YAV {
namespace Network {

QtUdpClient::QtUdpClient() {
    qDebug() << "QtUdpClient created";
}

QtUdpClient::~QtUdpClient() {
    shutdown();
    qDebug() << "QtUdpClient destroyed";
}

bool QtUdpClient::initialize(const std::string& local_address, uint16_t local_port) {
    if (is_initialized_) {
        shutdown();
    }

    QHostAddress address(QString::fromStdString(local_address));

    // Используем QUdpSocket::ShareAddress для возможности
    // отправки с одного сокета на разные адреса
    socket_.bind(address, local_port,
                 QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (socket_.state() == QUdpSocket::BoundState) {
        is_initialized_ = true;
        qDebug() << "QtUdpClient: initialized on"
                 << QString::fromStdString(local_address) << ":" << local_port;
        return true;
    }

    return false;
}

bool QtUdpClient::sendTo(const std::string& address, uint16_t port,
                         const std::vector<uint8_t>& data) {
    if (!is_initialized_) {
        qWarning() << "QtUdpClient: not initialized";
        return false;
    }

    QHostAddress dest_address(QString::fromStdString(address));
    qint64 bytes_sent = socket_.writeDatagram(
        reinterpret_cast<const char*>(data.data()),
        data.size(),
        dest_address,
        port
        );

    if (bytes_sent == static_cast<qint64>(data.size())) {
        return true;
    } else {
        qWarning() << "QtUdpClient: failed to send data to"
                   << QString::fromStdString(address) << ":" << port
                   << "error:" << socket_.errorString();
        return false;
    }
}

std::vector<uint8_t> QtUdpClient::receiveFrom(std::string& sender_address,
                                              uint16_t& sender_port,
                                              int timeout_ms) {
    std::vector<uint8_t> result;

    if (!is_initialized_) {
        return result;
    }

    // Ждём данные с таймаутом          ??мы должны отправлять данные с таймаутом. никого не ждем.
    // Если пакет просрочен что мы делаем? Надо игнорить? или мы обновляемся все равно?
    if (socket_.waitForReadyRead(timeout_ms)) {
        while (socket_.hasPendingDatagrams()) {
            QNetworkDatagram datagram = socket_.receiveDatagram();
            if (datagram.isValid()) {
                QByteArray data = datagram.data();
                result.assign(data.begin(), data.end());

                sender_address = datagram.senderAddress().toString().toStdString();
                sender_port = datagram.senderPort();
                break; // Берём только первый датаграм
            }
        }
    }

    return result;
}

void QtUdpClient::shutdown() {
    if (is_initialized_) {
        socket_.close();
        is_initialized_ = false;
        qDebug() << "QtUdpClient: shutdown";
    }
}

} // namespace Network
} // namespace YAV
