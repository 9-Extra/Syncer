#pragma once

#include <stdexcept>

namespace Syncer {

class SyncerException : public std::runtime_error
{
public:
    explicit SyncerException(const char* message)
        : std::runtime_error(message)
    {
    }

    explicit SyncerException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};


}