#ifndef NETWORKFACTORY_H
#define NETWORKFACTORY_H

#include "network.h"
#include <memory>

namespace YAV {
namespace Network {

// Фабрика для GUI части
std::unique_ptr<IUdpClient> createQtUdpClient();

} // namespace Network
} // namespace YAV

#endif // NETWORKFACTORY_H
