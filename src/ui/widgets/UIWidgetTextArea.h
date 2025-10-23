#ifndef UI_WIDGET_TEXTAREA_H
#define UI_WIDGET_TEXTAREA_H

/**
 * @brief Widget champ de saisie texte (textarea)
 * Support clavier virtuel, validation, placeholder
 */
class UIWidgetTextArea : public UIWidget {
private:
    lv_obj_t* textarea;
    lv_obj_t* keyboard;
    bool one_line;
    uint32_t max_length;
    
    // Callback externe pour validation
    void (*on_value_changed)(const char* text, void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void focusCallback(lv_event_t* e) {
        UIWidgetTextArea* self = (UIWidgetTextArea*)lv_event_get_user_data(e);
        if (self && self->keyboard) {
            lv_keyboard_set_textarea(self->keyboard, self->textarea);
            lv_obj_clear_flag(self->keyboard, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    static void valueChangedCallback(lv_event_t* e) {
        UIWidgetTextArea* self = (UIWidgetTextArea*)lv_event_get_user_data(e);
        if (self && self->on_value_changed) {
            const char* text = lv_textarea_get_text(self->textarea);
            self->on_value_changed(text, self->user_data);
        }
    }
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetTextArea(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          textarea(nullptr),
          keyboard(nullptr),
          one_line(true),
          max_length(64),
          on_value_changed(nullptr),
          user_data(nullptr) {}
    
    ~UIWidgetTextArea() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le textarea
     * @param parent Parent LVGL
     * @param width Largeur
     * @param height Hauteur (50 pour une ligne)
     * @param placeholder Texte placeholder
     * @param is_one_line true = une seule ligne
     * @param max_len Longueur max du texte
     */
    void create(lv_obj_t* parent, int width = 400, int height = 50,
               const char* placeholder = nullptr, bool is_one_line = true,
               uint32_t max_len = 64) {
        
        one_line = is_one_line;
        max_length = max_len;
        
        // Creer textarea
        textarea = lv_textarea_create(parent);
        container = textarea;
        
        lv_obj_set_size(textarea, width, height);
        lv_textarea_set_one_line(textarea, one_line);
        lv_textarea_set_max_length(textarea, max_length);
        
        if (placeholder) {
            lv_textarea_set_placeholder_text(textarea, placeholder);
        }
        
        // Style
        lv_obj_set_style_bg_color(textarea, lv_color_hex(0x0f1520), 0);
        lv_obj_set_style_border_color(textarea, lv_color_hex(0x4080a0), 0);
        lv_obj_set_style_border_width(textarea, 2, 0);
        lv_obj_set_style_radius(textarea, 8, 0);
        lv_obj_set_style_text_color(textarea, lv_color_white(), 0);
        lv_obj_set_style_text_font(textarea, &lv_font_montserrat_20, 0);
        lv_obj_set_style_pad_all(textarea, 8, 0);
        
        // Couleur curseur
        lv_obj_set_style_border_color(textarea, lv_color_hex(0x00d4ff), LV_PART_CURSOR);
        lv_obj_set_style_bg_color(textarea, lv_color_hex(0x00d4ff), LV_PART_CURSOR);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, 400, 50, "Enter text...", true, 64);
    }
    
    /**
     * @brief Associe un clavier virtuel
     * @param kb Objet clavier LVGL existant
     */
    void attachKeyboard(lv_obj_t* kb) {
        keyboard = kb;
        
        if (textarea && keyboard) {
            // Callback focus pour afficher clavier
            lv_obj_add_event_cb(textarea, focusCallback, LV_EVENT_FOCUSED, this);
        }
    }
    
    /**
     * @brief Definit callback de changement de valeur
     */
    void setOnValueChanged(void (*callback)(const char*, void*), void* data = nullptr) {
        on_value_changed = callback;
        user_data = data;
        
        if (textarea) {
            lv_obj_add_event_cb(textarea, valueChangedCallback, 
                              LV_EVENT_VALUE_CHANGED, this);
        }
    }
    
    /**
     * @brief Obtient le texte saisi
     */
    const char* getText() const {
        return textarea ? lv_textarea_get_text(textarea) : "";
    }
    
    /**
     * @brief Definit le texte
     */
    void setText(const char* text) {
        if (textarea) {
            lv_textarea_set_text(textarea, text);
        }
    }
    
    /**
     * @brief Efface le texte
     */
    void clear() {
        if (textarea) {
            lv_textarea_set_text(textarea, "");
        }
    }
    
    /**
     * @brief Definit le placeholder
     */
    void setPlaceholder(const char* placeholder) {
        if (textarea) {
            lv_textarea_set_placeholder_text(textarea, placeholder);
        }
    }
    
    /**
     * @brief Active/desactive le champ
     */
    void setEnabled(bool enabled) {
        if (textarea) {
            if (enabled) {
                lv_obj_clear_state(textarea, LV_STATE_DISABLED);
            } else {
                lv_obj_add_state(textarea, LV_STATE_DISABLED);
            }
        }
    }
    
    /**
     * @brief Configure mode mot de passe
     */
    void setPasswordMode(bool password) {
        if (textarea) {
            lv_textarea_set_password_mode(textarea, password);
        }
    }
    
    /**
     * @brief Configure acceptation caracteres
     */
    void setAcceptedChars(const char* accepted) {
        if (textarea) {
            lv_textarea_set_accepted_chars(textarea, accepted);
        }
    }
    
    /**
     * @brief Obtient l'objet textarea brut
     */
    lv_obj_t* getTextArea() const {
        return textarea;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Textarea = widget statique
    }
};

#endif