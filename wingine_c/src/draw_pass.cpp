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

    wg_semaphore_t* wg_draw_pass_create_on_finish_semaphore(wg_draw_pass_t* draw_pass) {
        return new wg_semaphore_t {
            .v = draw_pass->v->getSemaphores().createOnFinishSemaphore()
        };
    }

    void wg_draw_pass_set_wait_semaphores(wg_draw_pass_t* draw_pass, uint32_t num_semaphores, wg_semaphore_t** raw_semaphores) {
        std::vector<wg::Semaphore> semaphores(num_semaphores);
        for (uint32_t i = 0; i < num_semaphores; i++) {
            semaphores[i] = raw_semaphores[i]->v;
        }

        draw_pass->v->getSemaphores().setWaitSemaphores(semaphores);
    }

    void wg_draw_pass_render(wg_draw_pass_t* draw_pass) {
        draw_pass->v->render();
    }
};
