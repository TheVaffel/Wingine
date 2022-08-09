#pragma once

#include "./IUniformChain.hpp"
#include "./BasicUniform.hpp"

#include "../sync/SynchronizedQueueOperationBase.hpp"
#include "../util/IndexCounter.hpp"
#include "../core/ElementChainBase.hpp"

namespace wg::internal {

    template<typename T>
    class BasicUniformChain : public IUniformChain<T>, public ElementChainBase {
        std::vector<std::shared_ptr<IUniform<T>>> uniforms;

    public:
        BasicUniformChain(uint32_t count,
                          std::shared_ptr<const DeviceManager> device_manager);

        virtual void setAll(const T& value) override;
        virtual void setCurrent(const T& value) override;
        virtual IUniform<T>& getCurrentUniform() override;

        virtual IResource& getResourceAt(uint32_t index) override;
    };
};

#include "./BasicUniformChain.impl.hpp"
