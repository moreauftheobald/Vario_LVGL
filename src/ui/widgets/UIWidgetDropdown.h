#ifndef UI_WIDGET_DROPDOWN_H
#define UI_WIDGET_DROPDOWN_H

/**
 * @brief Widget liste deroulante (dropdown)
 * Support selection, callback, options dynamiques
 */
class UIWidgetDropdown : public UIWidget {
private:
    lv_obj_t* dropdown;
    
    // Callback externe
    void (*on_value_changed)(uint16_t selected_index, const char* selected_text, void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void valueChangedCallback(lv_event_t* e) {
        UIWidgetDropdown* self = (UIWidgetDropdown*)lv_event_get_user_data(e);
        if (self && self->on_value_changed && self->dropdown) {
            uint16_t selected = lv_dropdown_get_selected(self->dropdown);
            
            // Obtenir texte selectionne
            char buf[64];
            lv_dropdown_get_selected_str(self->dropdown, buf, sizeof(buf));
            
            self->on_value_changed(selected, buf, self->user_data);
        }
    }
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetDropdown(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          dropdown(nullptr),
          on_value_changed(nullptr),
          user_data(nullptr) {}
    
    ~UIWidgetDropdown() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le dropdown
     * @param parent Parent LVGL
     * @param options Options separees par \n (ex: "Option1\nOption2\nOption3")
     * @param width Largeur
     * @param height Hauteur
     */
    void create(lv_obj_t* parent, const char* options = "Option 1\nOption 2\nOption 3",
               int width = 400, int height = 50) {
        
        dropdown = lv_dropdown_create(parent);
        container = dropdown;
        
        lv_obj_set_size(dropdown, width, height);
        
        if (options) {
            lv_dropdown_set_options(dropdown, options);
        }
        
        // Style
        lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x0f1520), 0);
        lv_obj_set_style_border_color(dropdown, lv_color_hex(0x4080a0), 0);
        lv_obj_set_style_border_width(dropdown, 2, 0);
        lv_obj_set_style_radius(dropdown, 8, 0);
        lv_obj_set_style_text_color(dropdown, lv_color_white(), 0);
        lv_obj_set_style_text_font(dropdown, &lv_font_montserrat_20, 0);
        lv_obj_set_style_pad_all(dropdown, 8, 0);
        
        // Style liste deroulante
        lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x0f1520), LV_PART_SELECTED);
        lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x00d4ff), LV_STATE_CHECKED);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Option 1\nOption 2\nOption 3", 400, 50);
    }
    
    /**
     * @brief Definit les options
     * @param options Chaine avec options separees par \n
     */
    void setOptions(const char* options) {
        if (dropdown && options) {
            lv_dropdown_set_options(dropdown, options);
        }
    }
    
    /**
     * @brief Ajoute une option a la fin
     */
    void addOption(const char* option) {
        if (dropdown && option) {
            lv_dropdown_add_option(dropdown, option, LV_DROPDOWN_POS_LAST);
        }
    }
    
    /**
     * @brief Efface toutes les options
     */
    void clearOptions() {
        if (dropdown) {
            lv_dropdown_clear_options(dropdown);
        }
    }
    
    /**
     * @brief Definit l'option selectionnee par index
     */
    void setSelected(uint16_t index) {
        if (dropdown) {
            lv_dropdown_set_selected(dropdown, index);
        }
    }
    
    /**
     * @brief Obtient l'index de l'option selectionnee
     */
    uint16_t getSelected() const {
        return dropdown ? lv_dropdown_get_selected(dropdown) : 0;
    }
    
    /**
     * @brief Obtient le texte de l'option selectionnee
     */
    void getSelectedText(char* buf, uint32_t buf_size) const {
        if (dropdown && buf) {
            lv_dropdown_get_selected_str(dropdown, buf, buf_size);
        }
    }
    
    /**
     * @brief Definit callback de changement de valeur
     */
    void setOnValueChanged(void (*callback)(uint16_t, const char*, void*), void* data = nullptr) {
        on_value_changed = callback;
        user_data = data;
        
        if (dropdown) {
            lv_obj_add_event_cb(dropdown, valueChangedCallback, 
                              LV_EVENT_VALUE_CHANGED, this);
        }
    }
    
    /**
     * @brief Configure direction ouverture
     */
    void setDirection(lv_dir_t dir) {
        if (dropdown) {
            lv_dropdown_set_dir(dropdown, dir);
        }
    }
    
    /**
     * @brief Active/desactive
     */
    void setEnabled(bool enabled) {
        if (dropdown) {
            if (enabled) {
                lv_obj_clear_state(dropdown, LV_STATE_DISABLED);
            } else {
                lv_obj_add_state(dropdown, LV_STATE_DISABLED);
            }
        }
    }
    
    /**
     * @brief Obtient l'objet dropdown brut
     */
    lv_obj_t* getDropdown() const {
        return dropdown;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Dropdown = widget statique
    }
};

#endif