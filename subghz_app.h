#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <input/input.h>
#include <notification/notification.h>

#include "decoder.h"

#define APP_FREQ 433920000UL
#define APP_RX_TIMEOUT 500
#define MAX_MESSAGES 16
#define MAX_MESSAGE_LEN 64

typedef enum {
    AppViewMain,
    AppViewTextInput,
} AppView;

typedef struct {
    char text[MAX_MESSAGE_LEN];
    uint32_t timestamp;
} MessageEntry;

typedef struct {
    FuriMessageQueue* event_queue;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    Widget* main_widget;
    TextInput* text_input;
    NotificationApp* notifications;
    
    bool running;
    bool tx_mode;
    
    MessageEntry messages[MAX_MESSAGES];
    uint8_t message_count;
    
    char input_buffer[MAX_MESSAGE_LEN];
    char temp_buffer[MAX_MESSAGE_LEN];
} SubGhzApp;

typedef enum {
    EventTypeKey,
    EventTypeRxPacket,
    EventTypeExit,
} EventType;

typedef struct {
    EventType type;
    uint8_t data[128];
    uint8_t length;
    InputKey key;
} AppEvent;

void subghz_app_add_message(SubGhzApp* app, const char* msg);
void subghz_app_send_message(SubGhzApp* app, const char* msg);
void subghz_app_update_display(SubGhzApp* app);
