#ifndef UI_WIDGET_SEPARATOR_H
#define UI_WIDGET_SEPARATOR_H

/**
 * @brief Widget separateur (ligne horizontale ou verticale)
 * Simple ligne de separation entre elements
 */
class UIWidgetSeparator : public UIWidget {
private:
    lv_obj_t* separator;
    
public:
    /**
     * @brief Orientation du separateur
     */
    enum Orientation {
        HORIZONTAL,
        VERTICAL
    };
    
    /**
     * @brief Style de separateur
     */
    enum Style {
        STYLE_LIGHT,    // Gris clair
        STYLE_MEDIUM,   // Gris moyen
        STYLE_DARK,     // Gris fonce
        STYLE_ACCENT    // Couleur accent (bleu)
    };
    
    /**
     * @brief Constructeur
     */
    UIWidgetSeparator(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          separator(nullptr) {}
    
    ~UIWidgetSeparator() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree separateur horizontal personnalise
     */
    void createHorizontal(lv_obj_t* parent, lv_coord_t width = LV_PCT(100),
                         lv_coord_t height = 2, lv_color_t color = lv_color_hex(0x404040)) {
        separator = lv_obj_create(parent);
        container = separator;
        
        lv_obj_set_size(separator, width, height);
        lv_obj_set_style_bg_color(separator, color, 0);
        lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(separator, 0, 0);
        lv_obj_set_style_radius(separator, 0, 0);
        lv_obj_set_style_pad_all(separator, 0, 0);
        
        initialized = true;
    }
    
    /**
     * @brief Cree separateur vertical personnalise
     */
    void createVertical(lv_obj_t* parent, lv_coord_t width = 2,
                       lv_coord_t height = LV_PCT(100), lv_color_t color = lv_color_hex(0x404040)) {
        separator = lv_obj_create(parent);
        container = separator;
        
        lv_obj_set_size(separator, width, height);
        lv_obj_set_style_bg_color(separator, color, 0);
        lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(separator, 0, 0);
        lv_obj_set_style_radius(separator, 0, 0);
        lv_obj_set_style_pad_all(separator, 0, 0);
        
        initialized = true;
    }
    
    /**
     * @brief Cree separateur avec style predefini
     */
    void create(lv_obj_t* parent, Orientation orientation, Style style) {
        lv_color_t color;
        
        switch(style) {
            case STYLE_LIGHT:
                color = lv_color_hex(0x606060);
                break;
            
            case STYLE_MEDIUM:
                color = lv_color_hex(0x404040);
                break;
            
            case STYLE_DARK:
                color = lv_color_hex(0x202020);
                break;
            
            case STYLE_ACCENT:
                color = lv_color_hex(0x00d4ff);
                break;
            
            default:
                color = lv_color_hex(0x404040);
        }
        
        if (orientation == HORIZONTAL) {
            createHorizontal(parent, LV_PCT(100), 2, color);
        } else {
            createVertical(parent, 2, LV_PCT(100), color);
        }
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, HORIZONTAL, STYLE_MEDIUM);
    }
    
    /**
     * @brief Change la couleur du separateur
     */
    void setColor(lv_color_t color) {
        if (separator) {
            lv_obj_set_style_bg_color(separator, color, 0);
        }
    }
    
    /**
     * @brief Change l'epaisseur du separateur
     */
    void setThickness(lv_coord_t thickness, Orientation orientation) {
        if (separator) {
            if (orientation == HORIZONTAL) {
                lv_obj_set_height(separator, thickness);
            } else {
                lv_obj_set_width(separator, thickness);
            }
        }
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Separateur = element statique
    }
};

#endif