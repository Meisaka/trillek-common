#include "controllers/network/VMAC-datagram-hasher.hpp"

//#include <cstring>
//#include <iostream>
#include "algparam.h"
#include "argnames.h"

using namespace CryptoPP;

namespace trillek { namespace network { namespace cryptography {
VMAC_DatagramHasher::VMAC_DatagramHasher(buffer&& key)
    : _key(std::move(key)) {
    _hasher.SetKey(_key->data(), _key->size(),
        MakeParameters(Name::IV(), ConstByteArrayParameter(_nonce, 8, false), false));
};

bool VMAC_DatagramHasher::Verify(const byte* digest, const byte* message, size_t len, uint64_t nonce) {
    CryptoPP::Integer inonce(CryptoPP::Integer::POSITIVE, static_cast<CryptoPP::lword>(nonce));
/*   Integer vmac, k, m;
    vmac.Decode(digest, 8);
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << ">>Verify VMAC";
    std::cout << "   VMAC is " << std::hex << vmac;
    std::cout << "   nonce is " << _nonce;
    std::cout << "   key is " << std::hex << k;
    std::cout << "   message is " << std::hex << m;
    std::cout << "   len is " << len;
*/
    inonce.Encode(_nonce, 8);
    _hasher.Resynchronize(_nonce, 8);
    if (_hasher.VerifyDigest(digest, message, len)) {
//      std::cout << ">>VMAC2 OK";
        return true;
    }
//  std::cout << ">>VMAC2 FAILED";
    return false;
}

void VMAC_DatagramHasher::CalculateDigest(byte* digest, const byte* message, size_t len, uint64_t nonce) {
    CryptoPP::Integer inonce(CryptoPP::Integer::POSITIVE, static_cast<CryptoPP::lword>(nonce));
    inonce.Encode(_nonce, 8);
//  std::cout << ">>Compute VMAC";
    _hasher.Resynchronize(_nonce, 8);
    _hasher.CalculateDigest(digest, message, len);
/*  Integer vmac, nonc, k, m;
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << "   nonce is " << inonce;
    std::cout << "   key is " << std::hex << k;
    std::cout << "   message is " << std::hex << m;
    std::cout << "   len is " << len;
    _hasher.Info();
    vmac.Decode(digest, 8);
    std::cout << "   VMAC is " << std::hex << vmac;
*/
}
} // cryptography
} // network
} // trillek
