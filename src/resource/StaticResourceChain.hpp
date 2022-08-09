#include "./IResourceChain.hpp"
#include "../util/IndexCounter.hpp"

#include "../core/ElementChainBase.hpp"

namespace wg::internal {
    class StaticResourceChain : public virtual IResourceChain, public ElementChainBase {

        std::shared_ptr<IResource> resource;

    public:

        StaticResourceChain(uint32_t num_resources,
                            std::shared_ptr<IResource> resource);

        virtual IResource& getResourceAt(uint32_t index) final;
    };
};
