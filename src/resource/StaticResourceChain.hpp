#include "./IResourceChain.hpp"
#include "../util/IndexCounter.hpp"

namespace wg::internal {
    class StaticResourceChain : public virtual IResourceChain {

        IndexCounter index_counter;

        std::shared_ptr<IResource> resource;

    public:

        virtual void swap() final;
        virtual uint32_t getCurrentIndex() const final;
        virtual uint32_t getNumResources() const final;

        virtual IResource& getResourceAt(uint32_t index) final;

        StaticResourceChain(uint32_t num_resources,
                            std::shared_ptr<IResource> resource);
    };
};
