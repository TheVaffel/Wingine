#pragma once

#include "../resource/IResource.hpp"
#include "./IBuffer.hpp"

namespace wg::internal {
    class IStorageBuffer : public virtual IResource, public virtual IBuffer { };
};
