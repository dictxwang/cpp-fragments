#include <iostream>
#include <sodium.h>
#include <random>
#include <iomanip>

static std::string base64_encode(const unsigned char* data, size_t len) {
    static const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (size_t i = 0; i < len; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');
    return encoded;
}

static unsigned char* parse_private_key(std::string public_key, std::string private_key) {

    unsigned char pkey[crypto_sign_PUBLICKEYBYTES];
    unsigned char skey[crypto_sign_SECRETKEYBYTES];
    memcpy(pkey, public_key.c_str(), crypto_sign_PUBLICKEYBYTES);
    memcpy(skey, private_key.c_str(), crypto_sign_SECRETKEYBYTES);
    crypto_sign_keypair(pkey, skey);
    return skey;
}

static std::string sign_payload_by_ed25519(unsigned char* skey, std::string &payload) {
    
    unsigned char signature[crypto_sign_BYTES];
    crypto_sign_detached(signature,
                        nullptr, // Signature length (optional, not needed here)
                        (const unsigned char*)payload.c_str(),
                        payload.size(),
                        skey);

    // Encode the signature in Base64
    std::string base64_signature = base64_encode(signature, crypto_sign_BYTES);
    return base64_signature;
}

static std::string sign_by_ed25519(unsigned char* skey, std::string &payload) {
    
    std::vector<unsigned char> signed_message(crypto_sign_BYTES + payload.size());
    unsigned long long signed_message_len;

    crypto_sign(signed_message.data(), &signed_message_len,
                reinterpret_cast<const unsigned char*>(payload.data()), payload.size(),
                skey);

    // Encode the signature in Base64
    std::string base64_signature = base64_encode(signed_message.data(), crypto_sign_BYTES);
    return base64_signature;
}


int main(int argc, char const *argv[])
{
    // sodium_init();
    
    std::string ed25519_apikey = "";
    std::string ed25519_skey = "";
    
    std::string payload = "apiKey=&timestamp=1746605972672";

    unsigned char* signed_skey = parse_private_key(ed25519_apikey, ed25519_skey);
    std::string signature = sign_payload_by_ed25519(signed_skey, payload);

    std::cout << "Signature: " << signature << std::endl;

    std::string signature2 = sign_by_ed25519(signed_skey, payload);

    std::cout << "Signature: " << signature2 << std::endl;
    /* code */
    return 0;
}

