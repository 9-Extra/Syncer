#pragma once

#include <Base/DataChunk.h>

namespace Syncer {
class Encryptor {
public:
virtual ~Encryptor() = default;

virtual Syncer::DataChunk encrypt(Syncer::DataChunk src) = 0;
};


} // namespace Syncer
