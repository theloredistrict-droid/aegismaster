#include <android_native_app_glue.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AEGIS", __VA_ARGS__))

// Handle Android lifecycle commands
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            LOGI("AEGIS: Native Window Initialized.");
            break;
        case APP_CMD_TERM_WINDOW:
            LOGI("AEGIS: Native Window Terminated.");
            break;
    }
}

void android_main(struct android_app* state) {
    // Set command handler
    state->onAppCmd = engine_handle_cmd;

    LOGI("AEGIS Game Engine Initialized on Android");
    
    int ident;
    int events;
    struct android_poll_source* source;

    // Main Engine Event Loop
    while (true) {
        while ((ident = ALooper_pollAll(-1, nullptr, &events, (void**)&source)) >= 0) {
            if (source != nullptr) {
                source->process(state, source);
            }
            if (state->destroyRequested != 0) {
                return;
            }
        }
    }
}
