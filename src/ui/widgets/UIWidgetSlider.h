#ifndef UI_WIDGET_SLIDER_H
#define UI_WIDGET_SLIDER_H

/**
 * @brief Widget curseur de reglage (slider)
 * Support plage de valeurs, callback, label valeur
 */
class UIWidgetSlider : public UIWidget {
private:
    lv_obj_t* slider;
    lv_obj_t* value_label;
    
    int32_t min_value;
    int32_t max_value;
    bool show_value;
    
    // Callback externe
    void (*on_value_changed)(int32_t value, void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void valueChangedCallback(lv_event_t* e) {
        UIWidgetSlider* self = (UIWidgetSlider*)lv_event_get_user_data(e);
        if (self && self->slider) {
            int32_t value = lv_slider_get_value(self->slider);
            
            // Mettre a jour label si affiche
            if (self->show_value && self->value_label) {
                lv_label_set_text_fmt(self->value_label, "%d", (int)value);
            }
            
            // Appeler callback externe
            if (self->on_value_changed) {
                self->on_value_changed(value, self->user_data);
            }
        }
    }
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetSlider(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          slider(nullptr),
          value_label(nullptr),
          min_value(0),
          max_value(100),
          show_value(true),
          on_value_changed(nullptr),
          user_data(nullptr) {}
    
    ~UIWidgetSlider() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le slider
     * @param parent Parent LVGL
     * @param width Largeur
     * @param min Valeur min
     * @param max Valeur max
     * @param initial Valeur initiale
     * @param show_val Afficher label valeur
     */
    void create(lv_obj_t* parent, int width = 400,
               int32_t min = 0, int32_t max = 100, int32_t initial = 50,
               bool show_val = true) {
        
        min_value = min;
        max_value = max;
        show_value = show_val;
        
        // Conteneur pour slider + label
        container = lv_obj_create(parent);
        lv_obj_set_size(container, width, 60);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN,
                             LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Slider
        slider = lv_slider_create(container);
        lv_obj_set_size(slider, show_value ? width - 80 : width, 20);
        lv_slider_set_range(slider, min_value, max_value);
        lv_slider_set_value(slider, initial, LV_ANIM_OFF);
        
        // Style slider
        lv_obj_set_style_bg_color(slider, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(slider, 10, LV_PART_MAIN);
        
        // Style indicateur (partie remplie)
        lv_obj_set_style_bg_color(slider, lv_color_hex(0x00d4ff), LV_PART_INDICATOR);
        lv_obj_set_style_radius(slider, 10, LV_PART_INDICATOR);
        
        // Style knob (poignee)
        lv_obj_set_style_bg_color(slider, lv_color_hex(0x00d4ff), LV_PART_KNOB);
        lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
        lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        lv_obj_set_style_pad_all(slider, 6, LV_PART_KNOB);
        
        // Label valeur (optionnel)
        if (show_value) {
            value_label = lv_label_create(container);
            lv_label_set_text_fmt(value_label, "%d", (int)initial);
            lv_obj_set_style_text_font(value_label, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(value_label, lv_color_hex(0x00d4ff), 0);
            lv_obj_set_width(value_label, 60);
            lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, 0);
        }
        
        // Callback changement valeur
        lv_obj_add_event_cb(slider, valueChangedCallback, 
                          LV_EVENT_VALUE_CHANGED, this);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, 400, 0, 100, 50, true);
    }
    
    /**
     * @brief Definit la valeur
     */
    void setValue(int32_t value, bool animate = false) {
        if (slider) {
            lv_slider_set_value(slider, value, animate ? LV_ANIM_ON : LV_ANIM_OFF);
            
            // Mettre a jour label
            if (show_value && value_label) {
                lv_label_set_text_fmt(value_label, "%d", (int)value);
            }
        }
    }
    
    /**
     * @brief Obtient la valeur actuelle
     */
    int32_t getValue() const {
        return slider ? lv_slider_get_value(slider) : 0;
    }
    
    /**
     * @brief Definit la plage de valeurs
     */
    void setRange(int32_t min, int32_t max) {
        min_value = min;
        max_value = max;
        
        if (slider) {
            lv_slider_set_range(slider, min_value, max_value);
        }
    }
    
    /**
     * @brief Obtient min
     */
    int32_t getMin() const {
        return min_value;
    }
    
    /**
     * @brief Obtient max
     */
    int32_t getMax() const {
        return max_value;
    }
    
    /**
     * @brief Definit callback de changement de valeur
     */
    void setOnValueChanged(void (*callback)(int32_t, void*), void* data = nullptr) {
        on_value_changed = callback;
        user_data = data;
    }
    
    /**
     * @brief Active/desactive
     */
    void setEnabled(bool enabled) {
        if (slider) {
            if (enabled) {
                lv_obj_clear_state(slider, LV_STATE_DISABLED);
            } else {
                lv_obj_add_state(slider, LV_STATE_DISABLED);
            }
        }
    }
    
    /**
     * @brief Change la couleur de l'indicateur
     */
    void setColor(lv_color_t color) {
        if (slider) {
            lv_obj_set_style_bg_color(slider, color, LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(slider, color, LV_PART_KNOB);
            
            if (value_label) {
                lv_obj_set_style_text_color(value_label, color, 0);
            }
        }
    }
    
    /**
     * @brief Obtient l'objet slider brut
     */
    lv_obj_t* getSlider() const {
        return slider;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Slider = widget statique
    }
};

#endif