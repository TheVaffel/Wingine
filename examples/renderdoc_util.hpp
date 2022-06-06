#pragma once

#include "renderdoc_app.h"


namespace renderdoc {

    typedef RENDERDOC_API_1_1_2 API;

    API* init();
    void start_capture(API* api);
    void end_capture(API* api);
};
