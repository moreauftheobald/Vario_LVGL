#ifndef UI_WIDGET_CHECKBOX_H
#define UI_WIDGET_CHECKBOX_H

/**
 * @brief Widget case a cocher (checkbox)
 * Support label, callback, etats
 */
class UIWidgetCheckbox : public UIWidget {
private:
    lv_obj_t* checkbox;
    
    // Callback externe
    void (*on_state_changed)(bool checked, void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void stateChangedCallback(lv_event_t* e) {
        UIWidgetCheckbox* self = (UIWidgetCheckbox*)lv_event_get_user_data(e);
        if (self && self->checkbox && self->on_state_changed) {
            bool checked = (lv_obj_get_state(self->checkbox) & LV_STATE_CHECKED) != 0;
            self->on_state_changed(checked, self->user_data);
        }
    }
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetCheckbox(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          checkbox(nullptr),
          on_state_changed(nullptr),
          user_data(nullptr) {}
    
    ~UIWidgetCheckbox() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree la checkbox
     * @param parent Parent LVGL
     * @param text Texte a droite de la checkbox
     * @param checked Etat initial
     */
    void create(lv_obj_t* parent, const char* text = nullptr, bool checked = false) {
        
        checkbox = lv_checkbox_create(parent);
        container = checkbox;
        
        if (text) {
            lv_checkbox_set_text(checkbox, text);
        }
        
        // Style texte
        lv_obj_set_style_text_font(checkbox, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(checkbox, lv_color_white(), 0);
        
        // Style checkbox elle-meme
        lv_obj_set_style_bg_color(checkbox, lv_color_hex(0x2a3f5f), LV_PART_INDICATOR);
        lv_obj_set_style_border_color(checkbox, lv_color_hex(0x4080a0), LV_PART_INDICATOR);
        lv_obj_set_style_border_width(checkbox, 2, LV_PART_INDICATOR);
        lv_obj_set_style_radius(checkbox, 5, LV_PART_INDICATOR);
        
        // Style quand cochee
        lv_obj_set_style_bg_color(checkbox, lv_color_hex(0x00d4ff), 
                                  LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_border_color(checkbox, lv_color_hex(0x00d4ff), 
                                      LV_PART_INDICATOR | LV_STATE_CHECKED);
        
        // Etat initial
        if (checked) {
            lv_obj_add_state(checkbox, LV_STATE_CHECKED);
        }
        
        // Callback
        lv_obj_add_event_cb(checkbox, stateChangedCallback, 
                          LV_EVENT_VALUE_CHANGED, this);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Checkbox", false);
    }
    
    /**
     * @brief Change le texte
     */
    void setText(const char* text) {
        if (checkbox && text) {
            lv_checkbox_set_text(checkbox, text);
        }
    }
    
    /**
     * @brief Definit l'etat coche/decoche
     */
    void setChecked(bool checked) {
        if (checkbox) {
            if (checked) {
                lv_obj_add_state(checkbox, LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(checkbox, LV_STATE_CHECKED);
            }
        }
    }
    
    /**
     * @brief Obtient l'etat coche/decoche
     */
    bool isChecked() const {
        if (checkbox) {
            return (lv_obj_get_state(checkbox) & LV_STATE_CHECKED) != 0;
        }
        return false;
    }
    
    /**
     * @brief Toggle l'etat
     */
    void toggle() {
        setChecked(!isChecked());
    }
    
    /**
     * @brief Definit callback de changement d'etat
     */
    void setOnStateChanged(void (*callback)(bool, void*), void* data = nullptr) {
        on_state_changed = callback;
        user_data = data;
    }
    
    /**
     * @brief Active/desactive
     */
    void setEnabled(bool enabled) {
        if (checkbox) {
            if (enabled) {
                lv_obj_clear_state(checkbox, LV_STATE_DISABLED);
            } else {
                lv_obj_add_state(checkbox, LV_STATE_DISABLED);
            }
        }
    }
    
    /**
     * @brief Change la couleur quand cochee
     */
    void setColor(lv_color_t color) {
        if (checkbox) {
            lv_obj_set_style_bg_color(checkbox, color, 
                                      LV_PART_INDICATOR | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(checkbox, color, 
                                         LV_PART_INDICATOR | LV_STATE_CHECKED);
        }
    }
    
    /**
     * @brief Obtient l'objet checkbox brut
     */
    lv_obj_t* getCheckbox() const {
        return checkbox;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Checkbox = widget statique
    }
};

#endif