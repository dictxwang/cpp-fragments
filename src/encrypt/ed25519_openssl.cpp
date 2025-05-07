#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <cstring>

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

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

int main() {

    std::string ed25519_apikey = "";
    std::string ed25519_skey = "-----BEGIN PRIVATE KEY-----\n\n-----END PRIVATE KEY-----";
    
    // Initialize OpenSSL error strings
    ERR_load_crypto_strings();

    // Message to be signed
    const std::string payload = "apiKey=&timestamp=1746605972672";
    std::cout << "Payload: " << payload << std::endl;

    // Create a BIO memory buffer from the PEM string
    BIO* bio = BIO_new_mem_buf(ed25519_skey.c_str(), -1);
    if (!bio) {
        std::cerr << "Failed to create BIO!" << std::endl;
        handleErrors();
    }

    // Read the private key from the BIO
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);

    if (!pkey) {
        std::cerr << "Failed to load private key!" << std::endl;
        handleErrors();
    }

    // Get the corresponding public key
    unsigned char public_key[32];
    size_t public_key_len = sizeof(public_key);
    if (EVP_PKEY_get_raw_public_key(pkey, public_key, &public_key_len) <= 0) {
        handleErrors();
    }

    std::cout << "002" << std::endl;

    std::cout << "Public Key: ";
    for (size_t i = 0; i < public_key_len; ++i) {
        printf("%02x", public_key[i]);
    }
    std::cout << std::endl;

    // Create a context for signing
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        handleErrors();
    }

    // Initialize the signing operation
    if (EVP_DigestSignInit(md_ctx, NULL, NULL, NULL, pkey) <= 0) {
        handleErrors();
    }

    // Sign the message
    size_t signature_len = 0;
    if (EVP_DigestSign(md_ctx, NULL, &signature_len, (const unsigned char*)payload.data(), payload.size()) <= 0) {
        handleErrors();
    }

    std::vector<unsigned char> signature(signature_len);
    if (EVP_DigestSign(md_ctx, signature.data(), &signature_len, (const unsigned char*)payload.data(), payload.size()) <= 0) {
        handleErrors();
    }

    // Resize the signature to the actual size
    signature.resize(signature_len);

    std::cout << "Signature: ";
    for (unsigned char byte : signature) {
        printf("%02x", byte);
    }
    std::cout << std::endl;

    std::string base64_text = base64_encode(signature.data(), signature.size());
    std::cout << "Base64: " << base64_text << std::endl;

    // Clean up the signing context
    EVP_MD_CTX_free(md_ctx);

    
    // Verify the signature
    EVP_MD_CTX* verify_ctx = EVP_MD_CTX_new();
    if (!verify_ctx) {
        handleErrors();
    }

    // Initialize the verification operation
    if (EVP_DigestVerifyInit(verify_ctx, NULL, NULL, NULL, pkey) <= 0) {
        handleErrors();
    }

    // Verify the signature
    if (EVP_DigestVerify(verify_ctx, signature.data(), signature.size(), (const unsigned char*)payload.data(), payload.size()) == 1) {
        std::cout << "Signature verified successfully!" << std::endl;
    } else {
        std::cerr << "Signature verification failed!" << std::endl;
    }

    // Clean up the verification context
    EVP_MD_CTX_free(verify_ctx);

    // Free the key
    EVP_PKEY_free(pkey);

    // Cleanup OpenSSL error strings
    ERR_free_strings();

    return 0;
}
