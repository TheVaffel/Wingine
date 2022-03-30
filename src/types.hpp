
#include <memory>

#include "./framebuffer/IFramebuffer.hpp"
#include "./framebuffer/IFramebufferChain.hpp"

namespace wg {
    typedef std::unique_ptr<internal::IFramebuffer> Framebuffer;
    typedef std::shared_ptr<internal::IFramebufferChain> FramebufferChain;
}
