#include "encryptor.h"

namespace Syncer {

class NoEncryption final : public EncryptFactory {
public:
    using EncryptFactory::EncryptFactory;
    virtual std::string generate_public_key(const std::string &) override { return ""; }
    virtual std::unique_ptr<EncryptFactory::Encoder> build_encoder(const std::string &) override {
        return std::make_unique<EncoderDecoder>();
    };
    virtual std::unique_ptr<EncryptFactory::Decoder> build_decoder(const std::string &) override {
        return std::make_unique<EncoderDecoder>();
    };

private:
    class EncoderDecoder final : public EncryptFactory::Encoder, public EncryptFactory::Decoder {
    public:
        virtual DataChunk encode(const DataChunk &content) override { return content.clone(); }
        virtual DataChunk decode(const DataChunk &content) override { return content.clone(); }
    };
};

} // namespace Syncer