#ifndef UI_WIDGET_BUTTON_PAIR_H
#define UI_WIDGET_BUTTON_PAIR_H

/**
 * @brief Widget composite : paire de boutons Save/Cancel/Reset
 * Utilise pour formulaires et parametres
 */
class UIWidgetButtonPair : public UIWidget {
private:
    UIWidgetButton* btn_save;
    UIWidgetButton* btn_cancel;
    UIWidgetButton* btn_reset;
    
    lv_obj_t* container_buttons;
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetButtonPair(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          btn_save(nullptr),
          btn_cancel(nullptr),
          btn_reset(nullptr),
          container_buttons(nullptr) {}
    
    ~UIWidgetButtonPair() {
        // Supprimer les boutons
        if (btn_save) delete btn_save;
        if (btn_cancel) delete btn_cancel;
        if (btn_reset) delete btn_reset;
    }
    
    /**
     * @brief Cree la paire de boutons
     * @param parent Parent LVGL
     * @param save_text Texte bouton Save (nullptr = pas de bouton)
     * @param cancel_text Texte bouton Cancel (nullptr = pas de bouton)
     * @param reset_text Texte bouton Reset (nullptr = pas de bouton)
     * @param save_cb Callback Save
     * @param cancel_cb Callback Cancel
     * @param reset_cb Callback Reset
     * @param user_data Donnees utilisateur passees aux callbacks
     */
    void create(lv_obj_t* parent,
               const char* save_text = "Save",
               const char* cancel_text = "Cancel",
               const char* reset_text = nullptr,
               lv_event_cb_t save_cb = nullptr,
               lv_event_cb_t cancel_cb = nullptr,
               lv_event_cb_t reset_cb = nullptr,
               void* user_data = nullptr) {
        
        // Conteneur principal
        container = lv_obj_create(parent);
        lv_obj_set_size(container, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, -15);
        
        // Conteneur boutons (flex row)
        container_buttons = lv_obj_create(container);
        lv_obj_set_size(container_buttons, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(container_buttons, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container_buttons, 0, 0);
        lv_obj_set_style_pad_all(container_buttons, 0, 0);
        lv_obj_set_flex_flow(container_buttons, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container_buttons, LV_FLEX_ALIGN_CENTER,
                             LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(container_buttons, 20, 0);
        lv_obj_center(container_buttons);
        
        // Bouton Reset (a gauche si present)
        if (reset_text && reset_cb) {
            btn_reset = new UIWidgetButton();
            btn_reset->configure(reset_text, nullptr, lv_color_hex(0xff9500),
                                300, 70, reset_cb, user_data);
            btn_reset->create(container_buttons, reset_text, nullptr,
                            lv_color_hex(0xff9500));
        }
        
        // Bouton Cancel (centre/gauche)
        if (cancel_text && cancel_cb) {
            btn_cancel = new UIWidgetButton();
            btn_cancel->configure(cancel_text, nullptr, lv_color_hex(0xff3b30),
                                 300, 70, cancel_cb, user_data);
            btn_cancel->create(container_buttons, cancel_text, nullptr,
                             lv_color_hex(0xff3b30));
        }
        
        // Bouton Save (droite)
        if (save_text && save_cb) {
            btn_save = new UIWidgetButton();
            btn_save->configure(save_text, nullptr, lv_color_hex(0x34c759),
                               300, 70, save_cb, user_data);
            btn_save->create(container_buttons, save_text, nullptr,
                           lv_color_hex(0x34c759));
        }
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Save", "Cancel", nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    
    /**
     * @brief Active/desactive bouton Save
     */
    void setSaveEnabled(bool enabled) {
        if (btn_save) {
            btn_save->setEnabled(enabled);
        }
    }
    
    /**
     * @brief Active/desactive bouton Cancel
     */
    void setCancelEnabled(bool enabled) {
        if (btn_cancel) {
            btn_cancel->setEnabled(enabled);
        }
    }
    
    /**
     * @brief Active/desactive bouton Reset
     */
    void setResetEnabled(bool enabled) {
        if (btn_reset) {
            btn_reset->setEnabled(enabled);
        }
    }
    
    /**
     * @brief Change le texte du bouton Save
     */
    void setSaveText(const char* text) {
        if (btn_save) {
            btn_save->setText(text);
        }
    }
    
    /**
     * @brief Change le texte du bouton Cancel
     */
    void setCancelText(const char* text) {
        if (btn_cancel) {
            btn_cancel->setText(text);
        }
    }
    
    /**
     * @brief Change le texte du bouton Reset
     */
    void setResetText(const char* text) {
        if (btn_reset) {
            btn_reset->setText(text);
        }
    }
    
    /**
     * @brief Obtient le bouton Save
     */
    UIWidgetButton* getSaveButton() const {
        return btn_save;
    }
    
    /**
     * @brief Obtient le bouton Cancel
     */
    UIWidgetButton* getCancelButton() const {
        return btn_cancel;
    }
    
    /**
     * @brief Obtient le bouton Reset
     */
    UIWidgetButton* getResetButton() const {
        return btn_reset;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Widget composite statique
    }
};

#endif