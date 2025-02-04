#include "app.h"
#include "ui/app_ui.h"

#include "stream_input.h"
#include "stream_manager_internal.h"
#include "backend/input_manager.h"

bool stream_input_handle_key_event(stream_manager_t *manager, const SDL_KeyboardEvent *event) {
#ifdef SDL_WEBOS_SCANCODE_EXIT
    SDL_Keysym keysym = event->keysym;
    switch ((int) keysym.scancode) {
        case SDL_WEBOS_SCANCODE_EXIT: {
            if (event->state == SDL_RELEASED) {
                stream_manager_set_overlay_opened(manager, true);
            }
            return true;
        }
        case SDL_WEBOS_SCANCODE_BACK: {
            // TODO: Send ESC key
            return true;
        }
    }
#endif
    return true;
}

bool stream_input_handle_mouse_event(stream_manager_t *manager, const SDL_Event *event) {
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            if (input_manager_get_and_reset_mouse_movement(manager->app->input_manager)) {
                break;
            }
            if (manager->app->settings->relmouse) {
                IHS_SessionSendMouseMovement(manager->session, event->motion.xrel, event->motion.yrel);
            } else {
                int w, h;
                SDL_GetWindowSize(manager->app->ui->window, &w, &h);
                IHS_SessionSendMousePosition(manager->session, (float) event->motion.x / (float) w,
                                             (float) event->motion.y / (float) h);
            }
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            IHS_StreamInputMouseButton button = 0;
            switch (event->button.button) {
                case SDL_BUTTON_LEFT:
                    button = IHS_MOUSE_BUTTON_LEFT;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = IHS_MOUSE_BUTTON_RIGHT;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = IHS_MOUSE_BUTTON_MIDDLE;
                    break;
                case SDL_BUTTON_X1:
                    button = IHS_MOUSE_BUTTON_X1;
                    break;
                case SDL_BUTTON_X2:
                    button = IHS_MOUSE_BUTTON_X2;
                    break;
            }
            if (button != 0) {
                if (event->button.state == SDL_RELEASED) {
                    IHS_SessionSendMouseUp(manager->session, button);
                } else {
                    IHS_SessionSendMouseDown(manager->session, button);
                }
            }
            return true;
        }
        case SDL_MOUSEWHEEL: {
            Sint32 x = event->wheel.x, y = event->wheel.y;
            if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                x *= -1;
                y *= -1;
            }
            if (x != 0) {
                IHS_SessionSendMouseWheel(manager->session, x < 0 ? IHS_MOUSE_WHEEL_LEFT : IHS_MOUSE_WHEEL_RIGHT);
            }
            if (y != 0) {
                IHS_SessionSendMouseWheel(manager->session, y > 0 ? IHS_MOUSE_WHEEL_UP : IHS_MOUSE_WHEEL_DOWN);
            }
            return true;
        }
    }
    return false;
}