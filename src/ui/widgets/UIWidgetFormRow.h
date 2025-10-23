#ifndef UI_WIDGET_FORM_ROW_H
#define UI_WIDGET_FORM_ROW_H

/**
 * @brief Widget composite : ligne de formulaire (label + widget)
 * Facilite creation de formulaires avec alignement automatique
 */
class UIWidgetFormRow : public UIWidget {
private:
    UIWidgetLabel* label;
    lv_obj_t* input_container;
    lv_obj_t* row_container;
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetFormRow(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          label(nullptr),
          input_container(nullptr),
          row_container(nullptr) {}
    
    ~UIWidgetFormRow() {
        if (label) delete label;
    }
    
    /**
     * @brief Cree la ligne de formulaire
     * @param parent Parent LVGL
     * @param label_text Texte du label
     * @param label_width Largeur du label (pixels)
     * @param input_width Largeur zone input (pixels ou LV_PCT)
     */
    void create(lv_obj_t* parent, const char* label_text = "Label:",
               int label_width = 200, int input_width = LV_PCT(100)) {
        
        // Conteneur ligne (flex row)
        row_container = lv_obj_create(parent);
        container = row_container;
        
        lv_obj_set_size(row_container, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row_container, 0, 0);
        lv_obj_set_style_pad_all(row_container, 0, 0);
        lv_obj_set_flex_flow(row_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row_container, LV_FLEX_ALIGN_START,
                             LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row_container, 20, 0);
        
        // Label
        label = new UIWidgetLabel();
        label->create(row_container, label_text, UIWidgetLabel::STYLE_INFO);
        lv_obj_set_width(label->getContainer(), label_width);
        
        // Conteneur pour widget input (a remplir par utilisateur)
        input_container = lv_obj_create(row_container);
        lv_obj_set_size(input_container, input_width, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(input_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(input_container, 0, 0);
        lv_obj_set_style_pad_all(input_container, 0, 0);
        lv_obj_set_flex_flow(input_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(input_container, LV_FLEX_ALIGN_START,
                             LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Label:", 200, LV_PCT(100));
    }
    
    /**
     * @brief Change le texte du label
     */
    void setLabelText(const char* text) {
        if (label) {
            label->setText(text);
        }
    }
    
    /**
     * @brief Change la couleur du label
     */
    void setLabelColor(lv_color_t color) {
        if (label) {
            label->setColor(color);
        }
    }
    
    /**
     * @brief Obtient le conteneur input pour y ajouter widgets
     * C'est la ou l'utilisateur ajoute son textarea, dropdown, etc.
     */
    lv_obj_t* getInputContainer() const {
        return input_container;
    }
    
    /**
     * @brief Obtient le label
     */
    UIWidgetLabel* getLabel() const {
        return label;
    }
    
    /**
     * @brief Obtient le conteneur de la ligne complete
     */
    lv_obj_t* getRowContainer() const {
        return row_container;
    }
    
    /**
     * @brief Cache/affiche la ligne complete
     */
    void setVisible(bool visible) {
        if (row_container) {
            if (visible) {
                lv_obj_clear_flag(row_container, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(row_container, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Widget composite statique
    }
};

#endif