#include "./renderdoc_util.hpp"

#include <cassert>
#include <dlfcn.h>

namespace renderdoc {


    RENDERDOC_API_1_1_2 *rdoc_api = nullptr;

    API* init() {

        // At init, on linux/android.
        // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
        if (void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI) dlsym(mod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
            assert(ret == 1);
        }

        return rdoc_api;
    }

    void start_capture(API* api) {
        // To start a frame capture, call StartFrameCapture.
        // You can specify NULL, NULL for the device to capture on if you have only one device and
        // either no windows at all or only one window, and it will capture from that device.
        // See the documentation below for a longer explanation
        if (rdoc_api) rdoc_api->StartFrameCapture(nullptr, nullptr);
    }


    void end_capture(API* api) {
        // stop the capture
        if (rdoc_api) rdoc_api->EndFrameCapture(nullptr, nullptr);
    }
};
