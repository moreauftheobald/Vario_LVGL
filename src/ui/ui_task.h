#ifndef UI_TASK_H
#define UI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "UIManager.h"
#include "lvgl_port/lvgl_port.h"

static TaskHandle_t ui_task_handle = NULL;

static void ui_update_task(void* pvParameters) {
    UIManager* ui_mgr = UIManager::getInstance();
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    
    while (1) {
        if (lvgl_port_lock(50)) {
            ui_mgr->getCurrentScreen()->updateWidgets();
            lvgl_port_unlock();
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

static bool ui_task_start() {
    // Init LVGL
    // ... (code existant lvgl_port_init)
    
    // Init UI Manager
    if (lvgl_port_lock(1000)) {
        UIManager::getInstance()->init();
        lvgl_port_unlock();
    }
    
    // Creer task update
    BaseType_t ret = xTaskCreatePinnedToCore(
        ui_update_task,
        "ui_update",
        8192,
        NULL,
        UI_TASK_PRIORITY,
        &ui_task_handle,
        tskNO_AFFINITY
    );
    
    return (ret == pdPASS);
}

#endif