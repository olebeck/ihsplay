#include <stdlib.h>

#include <lvgl.h>
#include <src/draw/sdl/lv_draw_sdl.h>

#include "app_ui.h"
#include "launcher.h"
#include "lvgl/fonts/bootstrap-icons/regular.h"
#include "lvgl/keypad.h"
#include "lvgl/mouse.h"
#include "lvgl/theme.h"
#include "backend/stream_manager.h"

static void app_input_populate_group(app_ui_t *ui);

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp) {
    lv_draw_sdl_drv_param_t *param = disp->driver->user_data;
    app_ui_t *ui = calloc(1, sizeof(app_ui_t));
    ui->app = app;
    ui->window = param->user_data;
    ui->root = lv_disp_get_scr_act(disp);
    ui->fm = lv_fragment_manager_create(NULL);

    lv_group_t *group = lv_group_create();
    lv_group_set_editing(group, 0);
    lv_group_set_default(group);
    ui->group = group;
    _lv_ll_init(&ui->modal_groups, sizeof(lv_group_t *));

    ui->indev.mouse = app_lv_mouse_indev_init();
    ui->indev.keypad = app_indev_keypad_init(app);

    app_ui_fontset_set_default_size(ui, &ui->font);
    app_ui_fontset_set_default_size(ui, &ui->iconfont);

    app_ui_fontset_init_fc(&ui->font, "sans-serif");
    app_ui_fontset_init_mem(&ui->iconfont, "bootstrap-icons", ttf_bootstrap_icons_data,
                            ttf_bootstrap_icons_size);
    app_ui_fontset_apply_fallback(&ui->font, &ui->iconfont);

    lv_obj_set_style_bg_opa(ui->root, LV_OPA_0, 0);

    lv_style_init(&ui->styles.action_btn_label);
    lv_style_set_text_font(&ui->styles.action_btn_label, ui->iconfont.heading2);

    lv_theme_set_parent(&ui->theme, lv_disp_get_theme(disp));
    app_theme_init(&ui->theme, ui);
    lv_disp_set_theme(disp, &ui->theme);
    return ui;
}

void app_ui_created(app_ui_t *ui) {
    app_ui_push_fragment(ui, &launcher_fragment_class, NULL);
    app_ui_resized(ui, lv_obj_get_width(ui->root), lv_obj_get_height(ui->root));
}

void app_ui_destroy(app_ui_t *ui) {
    lv_group_set_default(NULL);
    lv_group_del(ui->group);
    _lv_ll_clear(&ui->modal_groups);

    lv_style_reset(&ui->styles.action_btn_label);

    app_ui_fontset_deinit(&ui->iconfont);
    app_ui_fontset_deinit(&ui->font);
    lv_fragment_manager_del(ui->fm);

    app_indev_keypad_deinit(ui->indev.keypad);
    app_lv_mouse_indev_deinit(ui->indev.mouse);

    app_theme_deinit(&ui->theme);

    free(ui);
}

void app_ui_resized(app_ui_t *ui, int width, int height) {
    stream_manager_set_viewport_size(ui->app->stream_manager, width, height);
}

void app_ui_set_ignore_keys(app_ui_t *ui, bool ignore) {
    app_indev_keypad_set_ignore(ui->indev.keypad, ignore);
}

lv_fragment_t *app_ui_create_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args) {
    app_ui_fragment_args_t fargs = {ui->app, args};
    return lv_fragment_create(cls, &fargs);
}

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args) {
    lv_fragment_t *f = app_ui_create_fragment(ui, cls, args);
    lv_fragment_manager_push(ui->fm, f, &ui->root);
    app_ui_update_nav_back(ui);
}

void app_ui_remove_fragment(app_ui_t *ui, lv_fragment_t *f) {
    lv_fragment_manager_remove(ui->fm, f);
    app_ui_update_nav_back(ui);
}

void app_ui_pop_top_fragment(app_ui_t *ui) {
    lv_fragment_manager_pop(ui->fm);
    app_ui_update_nav_back(ui);
}

bool app_ui_dispatch_event(app_ui_t *ui, app_event_type_t type, app_ui_event_data_t *data) {
    return lv_fragment_manager_send_event(ui->fm, type, data);
}

void app_ui_push_modal_group(app_ui_t *ui, lv_group_t *group) {
    LV_ASSERT_NULL(group);
    lv_group_t **tail = _lv_ll_ins_tail(&ui->modal_groups);
    *tail = group;
    app_input_populate_group(ui);
}

void app_ui_remove_modal_group(app_ui_t *ui, lv_group_t *group) {
    lv_group_t **node = NULL;
    _LV_LL_READ_BACK(&ui->modal_groups, node) {
        if (*node == group) break;
    }
    if (node) {
        _lv_ll_remove(&ui->modal_groups, node);
        lv_mem_free(node);
    }
    app_input_populate_group(ui);
}

lv_group_t *app_ui_get_input_group(app_ui_t *ui) {
    return ui->indev.keypad->group;
}

void app_ui_update_nav_back(app_ui_t *ui) {
    size_t stack_size = lv_fragment_manager_get_stack_size(ui->fm);
    LV_ASSERT(stack_size > 0);
    app_ui_set_handle_nav_back(ui, stack_size > 1);
}

static void app_input_populate_group(app_ui_t *ui) {
    lv_group_t *group = NULL;
    lv_group_t *const *tail = _lv_ll_get_tail(&ui->modal_groups);
    if (tail) {
        group = *tail;
    }
    if (!group) {
        group = ui->group;
    }
    if (!group) {
        group = lv_group_get_default();
    }
    lv_indev_set_group(ui->indev.keypad, group);
    lv_indev_set_group(ui->indev.mouse, group);
}