#ifndef UI_SCREEN_H
#define UI_SCREEN_H

/**
 * @brief Classe de base pour tous les ecrans
 * Gere l'affichage plein ecran et les widgets enfants
 */
class UIScreen : public UIBase {
protected:
    lv_obj_t* screen;                    // Objet screen LVGL
    std::vector<UIWidget*> widgets;      // Liste widgets
    TaskHandle_t update_task_handle;     // Handle task update (optionnel)
    
    /**
     * @brief Wrapper statique pour task FreeRTOS
     * Necessaire car FreeRTOS n'accepte que fonctions C
     */
    static void updateTaskWrapper(void* pvParameters) {
        UIScreen* self = static_cast<UIScreen*>(pvParameters);
        self->updateTask();
    }
    
    /**
     * @brief Boucle principale task update (optionnel)
     * Override si ecran necessite update periodique autonome
     */
    virtual void updateTask() {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10Hz
        
        while (1) {
            if (visible) {
                update();
                updateWidgets();
            }
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }
    
public:
    UIScreen() : UIBase(), screen(nullptr), update_task_handle(nullptr) {}
    
    virtual ~UIScreen() {
        // Supprimer task si existe
        if (update_task_handle) {
            vTaskDelete(update_task_handle);
            update_task_handle = nullptr;
        }
        
        // Supprimer widgets
        for (auto widget : widgets) {
            delete widget;
        }
        widgets.clear();
        
        // Supprimer screen
        if (screen) {
            lv_obj_del(screen);
            screen = nullptr;
        }
    }
    
    /**
     * @brief Cree l'ecran et son conteneur
     */
    void create(lv_obj_t* parent = nullptr) override {
        screen = lv_obj_create(parent);
        container = screen;
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        initialized = true;
    }
    
    /**
     * @brief Charge l'ecran (devient ecran actif)
     */
    virtual void load() {
        if (screen) {
            lv_scr_load(screen);
            show();
        }
    }
    
    /**
     * @brief Ajoute un widget a l'ecran
     * L'ecran prend possession du pointeur (delete auto)
     */
    void addWidget(UIWidget* widget) {
        if (widget) {
            widgets.push_back(widget);
        }
    }
    
    /**
     * @brief Met a jour tous les widgets visibles
     */
    void updateWidgets() {
        for (auto widget : widgets) {
            if (widget->isVisible()) {
                widget->update();
            }
        }
    }
    
    /**
     * @brief Demarre une task FreeRTOS dediee pour cet ecran
     * @param stack_size Taille stack (par defaut 4096)
     * @param priority Priorite task (par defaut UI_TASK_PRIORITY-1)
     * @param core_id Core affinity (tskNO_AFFINITY par defaut)
     */
    bool startUpdateTask(uint32_t stack_size = 4096, 
                        UBaseType_t priority = UI_TASK_PRIORITY - 1,
                        BaseType_t core_id = tskNO_AFFINITY) {
        if (update_task_handle) {
#ifdef DEBUG_MODE
            Serial.println("[UIScreen] Update task already running");
#endif
            return false;
        }
        
        BaseType_t ret = xTaskCreatePinnedToCore(
            updateTaskWrapper,
            "ui_screen_update",
            stack_size,
            this,
            priority,
            &update_task_handle,
            core_id
        );
        
#ifdef DEBUG_MODE
        if (ret == pdPASS) {
            Serial.println("[UIScreen] Update task created");
        } else {
            Serial.println("[UIScreen] Failed to create update task");
        }
#endif
        
        return (ret == pdPASS);
    }
    
    /**
     * @brief Arrete la task update si existe
     */
    void stopUpdateTask() {
        if (update_task_handle) {
            vTaskDelete(update_task_handle);
            update_task_handle = nullptr;
#ifdef DEBUG_MODE
            Serial.println("[UIScreen] Update task stopped");
#endif
        }
    }
};

#endif