#pragma once

#include "../base/DataChunk/DataChunk.h"
#include "../base/SyncerException.h"
#include <string>
#include <memory>
namespace Syncer {

class SyEncrytionException : public SyncerException
{
public:
    explicit SyEncrytionException(const char* message)
        : SyncerException(message)
    {
    }

    explicit SyEncrytionException(const std::string& message)
        : SyncerException(message)
    {
    }
};


class EncryptFactory{
public:  
    virtual std::string generate_public_key(const std::string& private_key) = 0;

    class Encoder{
    public:
        virtual DataChunk encode(const DataChunk& content) = 0;
        virtual ~Encoder() = default;
    };

    class Decoder{
    public:
        virtual DataChunk decode(const DataChunk& encoded_content) = 0;
        virtual ~Decoder() = default;
    };

    virtual std::unique_ptr<Encoder> build_encoder(const std::string& public_key) = 0;
    virtual std::unique_ptr<Decoder> build_decoder(const std::string& private_key) = 0;

    static EncryptFactory* find_encryptor(const std::string& name);
    static void init_encryptor_table();

    virtual ~EncryptFactory() = default;
};

}