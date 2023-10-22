#include "encryptor.h"
#include <algorithm>
#include <random>

namespace Syncer {

class Ks256 final : public EncryptFactory {
public:
    using EncryptFactory::EncryptFactory;
    virtual std::string generate_public_key(const std::string &private_key) override { return private_key; }
    virtual std::unique_ptr<EncryptFactory::Encoder> build_encoder(const std::string &public_key) override {
        return std::make_unique<Encoder>(public_key);
    };
    virtual std::unique_ptr<EncryptFactory::Decoder> build_decoder(const std::string &private_key) override {
        return std::make_unique<Decoder>(private_key);
    };

private:
    class Encoder final : public EncryptFactory::Encoder {
    public:
        Encoder(const std::string &public_key) {
            size_t seed = std::hash<std::string>()(public_key);
            for (size_t i = 0; i < 256; i++) {
                ks256_map[i] = i;
            }

            std::shuffle(ks256_map, ks256_map + 256, std::mt19937_64(seed));
        }

        virtual DataChunk encode(const DataChunk &content) override {
            DataChunk encoded(content.size);
            uint8_t* src_ptr = (uint8_t*)content.start;
            uint8_t* target_ptr = (uint8_t*)encoded.start;
            for (size_t i = 0; i < content.size; i++) {
                target_ptr[i] = ks256_map[src_ptr[i]];
            }
            return encoded;
        };

    private:
        uint8_t ks256_map[256];
    };

    class Decoder final : public EncryptFactory::Decoder {
    public:
        Decoder(const std::string &private_key) {
            // 凯撒加密是对称的，public key和private key相同
            uint8_t ks256_map[256];
            size_t seed = std::hash<std::string>()(private_key);
            for (size_t i = 0; i < 256; i++) {
                ks256_map[i] = i;
            }

            std::shuffle(ks256_map, ks256_map + 256, std::mt19937_64(seed));
            // 生成逆向映射表
            for (size_t i = 0; i < 256; i++) {
                inv_ks256_map[ks256_map[i]] = i;
            }
        }

        virtual DataChunk decode(const DataChunk &encoded_content) override {
            DataChunk decoded(encoded_content.size);
            uint8_t* src_ptr = (uint8_t*)encoded_content.start;
            uint8_t* target_ptr = (uint8_t*)decoded.start;
            for (size_t i = 0; i < encoded_content.size; i++) {
                target_ptr[i] = inv_ks256_map[src_ptr[i]];
            }
            return decoded;
        }

    private:
        uint8_t inv_ks256_map[256];
    };
};

} // namespace Syncer