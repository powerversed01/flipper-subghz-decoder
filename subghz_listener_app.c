#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_subghz.h>
#include <input/input.h>
#include <gui/gui.h>
#include <notification/notification.h>

#include "decoder.h"

#define LISTENER_FREQ 433920000UL
#define LISTENER_RX_TIMEOUT 1000

typedef struct {
    bool running;
    Gui* gui;
    ViewPort* view_port;
} SubGhzListenerApp;

static void render_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 12, AlignCenter, AlignTop, "SubGHz Listener");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignTop, "Listening...");
}

static void handle_packet(SubGhzListenerApp* app, const uint8_t* data, uint8_t len) {
    UNUSED(app);

    char decoded[64];
    if(decode_custom_packet(data, len, decoded, sizeof(decoded))) {
        FURI_LOG_I("SubGHz", "Decoded: %s", decoded);
    } else {
        FURI_LOG_I("SubGHz", "Packet received but not valid custom format");
    }
}

static void app_input_callback(InputEvent* input_event, void* context) {
    SubGhzListenerApp* app = context;

    if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        app->running = false;
    }
}

static void radio_worker(void* context) {
    SubGhzListenerApp* app = context;

    furi_hal_subghz_init();
    furi_hal_subghz_set_frequency(LISTENER_FREQ);
    furi_hal_subghz_set_modulation(FuriHalSubGhzModulationAsk);
    furi_hal_subghz_start();

    while(app->running) {
        uint8_t buffer[128];
        uint8_t length = 0;

        if(furi_hal_subghz_rx(buffer, sizeof(buffer), &length, LISTENER_RX_TIMEOUT) == 0) {
            continue;
        }

        if(length > 0) {
            handle_packet(app, buffer, length);
        }
    }

    furi_hal_subghz_stop();
    furi_hal_subghz_exit();
}

int32_t subghz_listener_app(void* p) {
    UNUSED(p);

    SubGhzListenerApp app = {0};
    app.running = true;

    app.gui = furi_record_open(RECORD_GUI);
    app.view_port = view_port_alloc();

    view_port_draw_callback_set(app.view_port, render_callback, &app);
    view_port_input_callback_set(app.view_port, app_input_callback, &app);

    Gui* gui = app.gui;
    gui_add_view_port(gui, app.view_port, GuiLayerFullscreen);

    FuriThread* thread = furi_thread_alloc();
    furi_thread_set_name(thread, "SubGHzListener");
    furi_thread_set_stack_size(thread, 4096);
    furi_thread_start(thread, radio_worker, &app);

    while(app.running) {
        view_port_update(app.view_port);
        furi_delay_ms(50);
    }

    furi_thread_join(thread);
    furi_thread_free(thread);

    gui_remove_view_port(gui, app.view_port);
    view_port_free(app.view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
