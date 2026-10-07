#include "subghz_app.h"

void subghz_app_add_message(SubGhzApp* app, const char* msg) {
    if(app->message_count >= MAX_MESSAGES) {
        for(int i = 0; i < MAX_MESSAGES - 1; i++) {
            furi_string_set(app->messages[i].text, app->messages[i + 1].text);
            app->messages[i].timestamp = app->messages[i + 1].timestamp;
        }
        app->message_count = MAX_MESSAGES - 1;
    }

    strncpy(app->messages[app->message_count].text, msg, MAX_MESSAGE_LEN - 1);
    app->messages[app->message_count].text[MAX_MESSAGE_LEN - 1] = '\0';
    app->messages[app->message_count].timestamp = furi_get_tick();
    app->message_count++;
}

void subghz_app_send_message(SubGhzApp* app, const char* msg) {
    UNUSED(app);
    FURI_LOG_I("SubGHz", "TX: %s", msg);
    // TX implementation would go here
}

void subghz_app_update_display(SubGhzApp* app) {
    UNUSED(app);
    // Display update would happen via GUI callbacks
}
