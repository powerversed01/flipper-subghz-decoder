#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_subghz.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <input/input.h>

#include "subghz_app.h"
#include "decoder.h"

static SubGhzApp* g_app = NULL;

static void main_view_draw_callback(Canvas* canvas, void* context) {
    SubGhzApp* app = context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "SubGHz TX/RX");

    canvas_set_font(canvas, FontSecondary);
    
    if(app->tx_mode) {
        canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignTop, "Mode: TX");
    } else {
        canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignTop, "Mode: RX");
    }

    canvas_draw_str_aligned(canvas, 2, 24, AlignLeft, AlignTop, "Messages:");

    uint8_t y_pos = 34;
    for(int i = app->message_count > 6 ? app->message_count - 6 : 0; i < app->message_count && y_pos < 60; i++) {
        canvas_draw_str_aligned(canvas, 4, y_pos, AlignLeft, AlignTop, app->messages[i].text);
        y_pos += 9;
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 2, 118, AlignLeft, AlignBottom, "OK: TX/RX | UP: Toggle | BACK: Exit");
}

static void main_view_input_callback(InputEvent* input_event, void* context) {
    SubGhzApp* app = context;

    if(input_event->type != InputTypeShort) {
        return;
    }

    switch(input_event->key) {
    case InputKeyUp:
        app->tx_mode = !app->tx_mode;
        break;
    case InputKeyOk:
        if(app->tx_mode) {
            view_dispatcher_send_navigation_event(app->view_dispatcher, ViewNavigationEventNext);
        }
        break;
    case InputKeyBack:
        app->running = false;
        break;
    default:
        break;
    }
}

static void text_input_callback(void* context) {
    SubGhzApp* app = context;
    
    if(strlen(app->temp_buffer) > 0) {
        subghz_app_send_message(app, app->temp_buffer);
        subghz_app_add_message(app, app->temp_buffer);
        memset(app->temp_buffer, 0, sizeof(app->temp_buffer));
    }

    view_dispatcher_send_navigation_event(app->view_dispatcher, ViewNavigationEventBack);
}

static void radio_worker(void* context) {
    SubGhzApp* app = context;

    furi_hal_subghz_init();
    furi_hal_subghz_set_frequency(APP_FREQ);
    furi_hal_subghz_set_modulation(FuriHalSubGhzModulationAsk);

    while(app->running) {
        if(!app->tx_mode) {
            furi_hal_subghz_start();

            uint8_t buffer[128];
            uint8_t length = 0;

            if(furi_hal_subghz_rx(buffer, sizeof(buffer), &length, APP_RX_TIMEOUT) > 0 && length > 0) {
                char decoded[MAX_MESSAGE_LEN];
                if(decode_custom_packet(buffer, length, decoded, sizeof(decoded))) {
                    AppEvent event = {.type = EventTypeRxPacket, .length = length};
                    memcpy(event.data, buffer, length);
                    furi_message_queue_put(app->event_queue, &event, FuriWaitForever);
                }
            }

            furi_hal_subghz_stop();
        } else {
            furi_delay_ms(100);
        }
    }

    furi_hal_subghz_exit();
}

static void app_event_handler(FuriMessageQueue* queue, void* context) {
    SubGhzApp* app = context;
    AppEvent event;

    while(furi_message_queue_get(queue, &event, 0) == FuriStatusOk) {
        if(event.type == EventTypeRxPacket) {
            char decoded[MAX_MESSAGE_LEN];
            if(decode_custom_packet(event.data, event.length, decoded, sizeof(decoded))) {
                FURI_LOG_I("SubGHz", "RX: %s", decoded);
                subghz_app_add_message(app, decoded);
            }
        }
    }
}

int32_t subghz_app_main(void* p) {
    UNUSED(p);

    SubGhzApp app = {0};
    g_app = &app;
    app.running = true;
    app.tx_mode = false;
    app.message_count = 0;

    app.event_queue = furi_message_queue_alloc(16, sizeof(AppEvent));
    app.gui = furi_record_open(RECORD_GUI);
    app.notifications = furi_record_open(RECORD_NOTIFICATION);

    app.view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app.view_dispatcher);
    view_dispatcher_attach_to_gui(app.view_dispatcher, app.gui, ViewDispatcherTypeFullscreen);

    app.main_widget = widget_alloc();
    widget_draw_set_callback(app.main_widget, main_view_draw_callback);
    widget_input_set_callback(app.main_widget, main_view_input_callback);
    view_dispatcher_add_view(app.view_dispatcher, AppViewMain, widget_get_view(app.main_widget));

    app.text_input = text_input_alloc();
    text_input_set_header_text(app.text_input, "Enter message:");
    text_input_set_result_callback(app.text_input, text_input_callback, &app, app.temp_buffer, MAX_MESSAGE_LEN, false);
    view_dispatcher_add_view(app.view_dispatcher, AppViewTextInput, text_input_get_view(app.text_input));

    view_dispatcher_switch_to_view(app.view_dispatcher, AppViewMain);

    FuriThread* radio_thread = furi_thread_alloc();
    furi_thread_set_name(radio_thread, "SubGHzRadio");
    furi_thread_set_stack_size(radio_thread, 4096);
    furi_thread_start(radio_thread, radio_worker, &app);

    while(app.running) {
        app_event_handler(app.event_queue, &app);
        furi_delay_ms(50);
    }

    furi_thread_join(radio_thread);
    furi_thread_free(radio_thread);

    view_dispatcher_remove_view(app.view_dispatcher, AppViewMain);
    view_dispatcher_remove_view(app.view_dispatcher, AppViewTextInput);
    widget_free(app.main_widget);
    text_input_free(app.text_input);
    view_dispatcher_free(app.view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(app.event_queue);

    return 0;
}
