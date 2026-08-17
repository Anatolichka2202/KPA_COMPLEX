#ifndef QTUDPCLIENT_H
#define QTUDPCLIENT_H

#include "network.h"
#include <QUdpSocket>
#include <QObject>
#include <memory>

namespace YAV {
namespace Network {

class QtUdpClient : public QObject, public IUdpClient {
    Q_OBJECT

public:
    QtUdpClient();
    ~QtUdpClient() override;

    // IUdpClient interface
    bool initialize(const std::string& local_address, uint16_t local_port) override;
    bool sendTo(const std::string& address, uint16_t port,
                const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> receiveFrom(std::string& sender_address,
                                     uint16_t& sender_port,
                                     int timeout_ms = 100) override;
    bool isInitialized() const override { return is_initialized_; }
    void shutdown() override;

private:
    QUdpSocket socket_;
    bool is_initialized_ = false;
};

} // namespace Network
} // namespace YAV

#endif // QTUDPCLIENT_H
