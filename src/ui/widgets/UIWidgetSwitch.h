#ifndef UI_WIDGET_SWITCH_H
#define UI_WIDGET_SWITCH_H

/**
 * @brief Widget interrupteur on/off (switch)
 * Similaire a checkbox mais style toggle moderne
 */
class UIWidgetSwitch : public UIWidget {
private:
    lv_obj_t* sw;
    lv_obj_t* label;
    
    // Callback externe
    void (*on_state_changed)(bool state, void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void stateChangedCallback(lv_event_t* e) {
        UIWidgetSwitch* self = (UIWidgetSwitch*)lv_event_get_user_data(e);
        if (self && self->sw && self->on_state_changed) {
            bool state = (lv_obj_get_state(self->sw) & LV_STATE_CHECKED) != 0;
            self->on_state_changed(state, self->user_data);
        }
    }
    
public:
    /**
     * @brief Position du label
     */
    enum LabelPosition {
        LABEL_LEFT,
        LABEL_RIGHT,
        LABEL_NONE
    };
    
    /**
     * @brief Constructeur
     */
    UIWidgetSwitch(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          sw(nullptr),
          label(nullptr),
          on_state_changed(nullptr),
          user_data(nullptr) {}
    
    ~UIWidgetSwitch() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le switch
     * @param parent Parent LVGL
     * @param text Texte du label (nullptr = pas de label)
     * @param label_pos Position du label
     * @param initial_state Etat initial (true = ON)
     */
    void create(lv_obj_t* parent, const char* text = nullptr,
               LabelPosition label_pos = LABEL_RIGHT, bool initial_state = false) {
        
        // Conteneur pour switch + label
        container = lv_obj_create(parent);
        lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START,
                             LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(container, 15, 0);
        
        // Label a gauche si demande
        if (text && label_pos == LABEL_LEFT) {
            label = lv_label_create(container);
            lv_label_set_text(label, text);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
        }
        
        // Switch
        sw = lv_switch_create(container);
        
        // Style switch OFF
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_MAIN);
        
        // Style switch ON
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x00d4ff), 
                                  LV_PART_INDICATOR | LV_STATE_CHECKED);
        
        // Style knob (bouton mobile)
        lv_obj_set_style_bg_color(sw, lv_color_white(), LV_PART_KNOB);
        lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        
        // Label a droite si demande
        if (text && label_pos == LABEL_RIGHT) {
            label = lv_label_create(container);
            lv_label_set_text(label, text);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
        }
        
        // Etat initial
        if (initial_state) {
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        }
        
        // Callback
        lv_obj_add_event_cb(sw, stateChangedCallback, 
                          LV_EVENT_VALUE_CHANGED, this);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Switch", LABEL_RIGHT, false);
    }
    
    /**
     * @brief Change le texte du label
     */
    void setText(const char* text) {
        if (label && text) {
            lv_label_set_text(label, text);
        }
    }
    
    /**
     * @brief Definit l'etat ON/OFF
     */
    void setState(bool state) {
        if (sw) {
            if (state) {
                lv_obj_add_state(sw, LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(sw, LV_STATE_CHECKED);
            }
        }
    }
    
    /**
     * @brief Obtient l'etat ON/OFF
     */
    bool getState() const {
        if (sw) {
            return (lv_obj_get_state(sw) & LV_STATE_CHECKED) != 0;
        }
        return false;
    }
    
    /**
     * @brief Toggle l'etat
     */
    void toggle() {
        setState(!getState());
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
        if (sw) {
            if (enabled) {
                lv_obj_clear_state(sw, LV_STATE_DISABLED);
            } else {
                lv_obj_add_state(sw, LV_STATE_DISABLED);
            }
        }
    }
    
    /**
     * @brief Change la couleur ON
     */
    void setColor(lv_color_t color) {
        if (sw) {
            lv_obj_set_style_bg_color(sw, color, 
                                      LV_PART_INDICATOR | LV_STATE_CHECKED);
        }
    }
    
    /**
     * @brief Obtient l'objet switch brut
     */
    lv_obj_t* getSwitch() const {
        return sw;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Switch = widget statique
    }
};

#endif