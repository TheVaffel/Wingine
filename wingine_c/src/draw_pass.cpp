#include "./draw_pass.h"

#include <Wingine.hpp>

#include "./types.hpp"
#include "./wingine.h"
#include "./pipeline.h"

extern "C" {
    wg_draw_pass_settings_t wg_default_draw_pass_settings() {

        return wg_draw_pass_settings_t {
            .render_pass_settings = wg_command_render_pass_settings_t {
                .num_color_attachments = 1,
                .num_depth_attachments = 1,

                .should_clear_color = 1,
                .should_clear_depth = 1,
                .finalize_as_texture = 0
            }
        };
    }


    wg_draw_pass_t* wg_create_draw_pass(wg_wingine_t* wing,
                                        wg_pipeline_t* pipeline,
                                        wg_draw_pass_settings_t raw_settings) {
        static_assert(sizeof(wg_draw_pass_settings_t) == sizeof(wg::BasicDrawPassSettings));
        wg::BasicDrawPassSettings& settings = *(wg::BasicDrawPassSettings*)&raw_settings;

        return new wg_draw_pass_t {
            .v = wing->wingine.createBasicDrawPass(pipeline->v, settings)
        };
    }

    void wg_destroy_draw_pass(wg_draw_pass_t* draw_pass) {
        delete draw_pass;
    }
};
