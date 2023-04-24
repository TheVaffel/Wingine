#pragma once

#include "./IRawUniformChain.hpp"
#include "./BasicRawUniform.hpp"

#include "../sync/SynchronizedQueueOperationBase.hpp"
#include "../util/IndexCounter.hpp"
#include "../core/ElementChainBase.hpp"

namespace wg::internal {

    class BasicRawUniformChain : public IRawUniformChain, public ElementChainBase {
        std::vector<std::shared_ptr<IRawUniform>> uniforms;

    public:
        BasicRawUniformChain(uint32_t count,
                          uint32_t byte_size,
                          std::shared_ptr<const DeviceManager> device_manager);

        virtual void setAll(const void* ptr) override;
        virtual void setCurrent(const void* ptr) override;
        virtual IRawUniform& getCurrentUniform() override;

        virtual IResource& getResourceAt(uint32_t index) override;

        virtual ~BasicRawUniformChain() = default;
    };
};
