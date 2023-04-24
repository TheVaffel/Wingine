#pragma once

namespace wg::internal {
    class RawUniform : public IResource {
    public:
        RawUniform(std::shared_ptr<const DeviceManager> device_manager);

        void set(unsigned char* data, uint32_t byte_size);

        virtual

        virtual ~RawUniform();
    };
};
