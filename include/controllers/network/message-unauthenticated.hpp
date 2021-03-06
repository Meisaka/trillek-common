#ifndef MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
#define MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek {
namespace network {

class MessageUnauthenticated final : public Message {
public:
    MessageUnauthenticated(vector_type&& buffer, size_t size,
        const ConnectionData* cnxd, socket_t fd)
        : Message(buffer.data(), size, cnxd),
          buffer(std::move(buffer)), cx_data(cnxd), fd(fd) {}

    ~MessageUnauthenticated() {}

    /** \brief File Descriptor getter
     *
     * \return the file descriptor
     *
     */
    socket_t FileDescriptor() const { return fd; }

    const ConnectionData* CxData() { return cx_data; }
private:
    const vector_type buffer;
    const ConnectionData* const cx_data;
    const socket_t fd;
};

} // network
} // trillek
#endif // MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
