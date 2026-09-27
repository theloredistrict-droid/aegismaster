#include <android_native_app_glue.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AEGIS", __VA_ARGS__))

void android_main(struct android_app* state) {
    LOGI("AEGIS Game Engine Initialized on Android");
    
    int ident;
    int events;
    struct android_poll_source* source;

    // Standard Android C++ Event Loop
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
