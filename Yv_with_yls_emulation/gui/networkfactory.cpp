#include "networkfactory.h"
#include "qtudpclient.h"

namespace YAV {
namespace Network {

std::unique_ptr<IUdpClient> createQtUdpClient() {
    return std::make_unique<QtUdpClient>();
}

} // namespace Network
} // namespace YAV
