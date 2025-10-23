#ifndef UI_WIDGET_BUTTON_H
#define UI_WIDGET_BUTTON_H

/**
 * @brief Widget bouton stylise avec icone et texte
 * Support des callbacks et personnalisation complete
 */
class UIWidgetButton : public UIWidget {
private:
    lv_obj_t* btn;
    lv_obj_t* icon_label;
    lv_obj_t* text_label;
    
    lv_event_cb_t callback;
    void* user_data;
    
    lv_color_t color;
    int width;
    int height;
    
public:
    /**
     * @brief Constructeur
     * @param update_interval_ms Intervalle MAJ (defaut: pas de MAJ auto)
     */
    UIWidgetButton(uint16_t update_interval_ms = 0) 
        : UIWidget(update_interval_ms),
          btn(nullptr),
          icon_label(nullptr),
          text_label(nullptr),
          callback(nullptr),
          user_data(nullptr),
          color(lv_color_hex(0x007aff)),
          width(200),
          height(80) {}
    
    ~UIWidgetButton() {
        // Cleanup auto par UIWidget
    }
    
    /**
     * @brief Configure le bouton AVANT create()
     */
    void configure(const char* text, const char* icon, 
                  lv_color_t btn_color, int btn_width, int btn_height,
                  lv_event_cb_t cb = nullptr, void* data = nullptr) {
        color = btn_color;
        width = btn_width;
        height = btn_height;
        callback = cb;
        user_data = data;
    }
    
    /**
     * @brief Cree le bouton
     * @param parent Parent LVGL
     * @param text Texte du bouton
     * @param icon Icone (symbole LVGL ou nullptr)
     * @param btn_color Couleur de fond
     */
    void create(lv_obj_t* parent, const char* text = nullptr, 
               const char* icon = nullptr, lv_color_t btn_color = lv_color_hex(0x007aff)) {
        
        color = btn_color;
        
        // Creer bouton
        btn = lv_button_create(parent);
        container = btn;
        lv_obj_set_size(btn, width, height);
        
        // Style
        lv_obj_set_style_bg_color(btn, color, 0);
        lv_obj_set_style_radius(btn, 15, 0);
        lv_obj_set_style_shadow_width(btn, 5, 0);
        lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
        
        // Style pressed (plus sombre)
        lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);
        
        // Icone (a gauche)
        if (icon) {
            icon_label = lv_label_create(btn);
            lv_label_set_text(icon_label, icon);
            lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_32, 0);
            lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
            lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 20, 0);
        }
        
        // Texte (centre ou decale si icone)
        if (text) {
            text_label = lv_label_create(btn);
            lv_label_set_text(text_label, text);
            lv_obj_set_style_text_font(text_label, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
            
            if (icon) {
                lv_obj_align(text_label, LV_ALIGN_LEFT_MID, 80, 0);
            } else {
                lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
            }
        }
        
        // Callback
        if (callback) {
            lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
        }
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        // Version par defaut (bouton generique)
        create(parent, "Button", nullptr, lv_color_hex(0x007aff));
    }
    
    /**
     * @brief Change le texte du bouton
     */
    void setText(const char* text) {
        if (text_label) {
            lv_label_set_text(text_label, text);
        }
    }
    
    /**
     * @brief Change l'icone du bouton
     */
    void setIcon(const char* icon) {
        if (icon_label) {
            lv_label_set_text(icon_label, icon);
        }
    }
    
    /**
     * @brief Change la couleur du bouton
     */
    void setColor(lv_color_t new_color) {
        color = new_color;
        if (btn) {
            lv_obj_set_style_bg_color(btn, color, 0);
            lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);
        }
    }
    
    /**
     * @brief Active/desactive le bouton
     */
    void setEnabled(bool enabled) {
        if (btn) {
            if (enabled) {
                lv_obj_clear_state(btn, LV_STATE_DISABLED);
                lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            } else {
                lv_obj_add_state(btn, LV_STATE_DISABLED);
                lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
            }
        }
    }
    
    /**
     * @brief Pas de MAJ periodique pour un bouton
     */
    void update() override {
        // Bouton = widget statique
    }
};

#endif