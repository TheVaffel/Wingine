#pragma once

#include "./IUniformChain.hpp"
#include "./BasicUniform.hpp"

#include "../sync/SynchronizedQueueOperationBase.hpp"
#include "../util/IndexCounter.hpp"

namespace wg::internal {

    template<typename T>
    class BasicUniformChain : public IUniformChain<T> {

        IndexCounter uniform_index;

        std::vector<std::shared_ptr<IUniform<T>>> uniforms;

    public:
        BasicUniformChain(uint32_t count,
                          std::shared_ptr<const DeviceManager> device_manager);

        virtual void setAllUniforms(const T& value) override;
        virtual void setCurrentUniform(const T& value) override;
        virtual IUniform<T>& getCurrentUniform() override;

        virtual uint32_t getCurrentIndex() const override;
        virtual uint32_t getNumResources() const override;
        virtual void swap() override;
        virtual IResource& getResourceAt(uint32_t index) override;
    };
};

#include "./BasicUniformChain.impl.hpp"
