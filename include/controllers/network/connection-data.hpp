#ifndef CONNECTIONDATA_H_INCLUDED
#define CONNECTIONDATA_H_INCLUDED

#include <atomic>
#include <vector>
#include "controllers/network/authentication-handler.hpp"

namespace trillek {
namespace network {

class NetworkNodeData;

/** \brief This object is attached to each socket
 */
class ConnectionData final {
public:
    /** \brief Constructor
     *
     * \param state const uint8_t the initial state of the client
     * \param connection the connection instance
     *
     */
    ConnectionData(const uint8_t state, std::shared_ptr<NetworkNodeData> node_data) :
        _auth_state(state),
        _node_data(node_data) {}

    /** \brief Constructor
     *
     * Note that the connection is considered authenticated
     *
     * \param id const id_t the id of the entity
     * \param verifier the verifier functor to check packet received
     */
    ConnectionData(std::shared_ptr<NetworkNodeData> node_data)
        : _auth_state(AUTH_SHARE_KEY), _node_data(node_data) {}

    ~ConnectionData() {}

    ConnectionData(const ConnectionData& that)
        : _auth_state(that._auth_state.load()), _node_data(that._node_data) {}

    /** \brief Compare atomically the current state of the connection
     *
     * \param state uint8_t the state to compare with
     * \return bool true if equal, false otherwise
     */
    bool CompareAuthState(uint8_t state) const {
        return (_auth_state.load() == state);
    }

    /** \brief Set atomically the state of the connection
     *
     * The previous state is checked for allowed transitions
     * AUTH_NONE is always allowed.
     *
     * true is returned to the first thread calling the transition
     * false is returned to other threads for the same transition
     *
     * \param state unsigned char the state to set
     * \return bool true if the new state was allowed and set
     */
    bool SetAuthState(uint8_t state) const {
        if (! state) {
            // AUTH_NONE
            _auth_state.store(state);
            return false;
        }
        auto previous = const_cast<uint8_t*>(&_states.at(state - 1));
        return std::atomic_compare_exchange_strong(&_auth_state, previous, state);
    }

    /** \brief Return the state of the connection
     *
     * \return uint8_t the state
     */
    uint8_t AuthState() const {
        return _auth_state.load();
    }

    /** \brief Get the instance of TCPConnection
     *
     * \return TCPConnection the connection
     */
    bool ConnectionAccept() const {
        uint8_t key_exchange_state = AUTH_KEY_EXCHANGE;
        uint8_t share_key_state = AUTH_SHARE_KEY;
        return (std::atomic_compare_exchange_strong(&_auth_state, &key_exchange_state, share_key_state));
    }

    std::shared_ptr<NetworkNodeData> GetNodeData() const { return _node_data; }

private:
    // connection write is protected by _auth_state single-threaded transition. no read
    mutable std::atomic<uint8_t> _auth_state;
    std::shared_ptr<NetworkNodeData> _node_data;
    static const std::vector<uint8_t> _states;
};
} // network
} // trillek
#endif // CONNECTIONDATA_H_INCLUDED
