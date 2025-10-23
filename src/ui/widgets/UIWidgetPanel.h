#ifndef UI_WIDGET_PANEL_H
#define UI_WIDGET_PANEL_H

/**
 * @brief Widget panel (conteneur) stylise
 * Support bordure, ombre, flex layout
 */
class UIWidgetPanel : public UIWidget {
private:
    lv_obj_t* panel;
    
public:
    /**
     * @brief Styles predéfinis de panel
     */
    enum Style {
        STYLE_DEFAULT,      // Noir avec bordure blanche
        STYLE_INFO,         // Fond semi-transparent bleu
        STYLE_DARK,         // Noir opaque sans bordure
        STYLE_CARD,         // Style carte avec ombre
        STYLE_BORDERED      // Bordure coloree
    };
    
    /**
     * @brief Constructeur
     */
    UIWidgetPanel(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          panel(nullptr) {}
    
    ~UIWidgetPanel() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree panel personnalise
     */
    void create(lv_obj_t* parent, int width, int height,
               lv_color_t bg_color = lv_color_hex(0x000000),
               lv_color_t border_color = lv_color_hex(0xFFFFFF),
               int border_width = 3, int radius = 15) {
        
        panel = lv_obj_create(parent);
        container = panel;
        
        lv_obj_set_size(panel, width, height);
        
        // Background
        lv_obj_set_style_bg_color(panel, bg_color, 0);
        lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
        
        // Bordure
        lv_obj_set_style_border_width(panel, border_width, 0);
        lv_obj_set_style_border_color(panel, border_color, 0);
        
        // Radius
        lv_obj_set_style_radius(panel, radius, 0);
        
        // Padding
        lv_obj_set_style_pad_all(panel, 20, 0);
        
        // Desactiver scroll par defaut
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
        
        initialized = true;
    }
    
    /**
     * @brief Cree panel avec style predefini
     */
    void create(lv_obj_t* parent, int width, int height, Style style) {
        lv_color_t bg;
        lv_color_t border;
        int border_w;
        int rad;
        
        switch(style) {
            case STYLE_DEFAULT:
                bg = lv_color_hex(0x000000);
                border = lv_color_hex(0xFFFFFF);
                border_w = 3;
                rad = 15;
                break;
            
            case STYLE_INFO:
                bg = lv_color_hex(0x1a2035);
                border = lv_color_hex(0x6080a0);
                border_w = 2;
                rad = 15;
                break;
            
            case STYLE_DARK:
                bg = lv_color_hex(0x000000);
                border = lv_color_hex(0x000000);
                border_w = 0;
                rad = 15;
                break;
            
            case STYLE_CARD:
                bg = lv_color_hex(0x1a1f3a);
                border = lv_color_hex(0x000000);
                border_w = 0;
                rad = 20;
                break;
            
            case STYLE_BORDERED:
                bg = lv_color_hex(0x000000);
                border = lv_color_hex(0x00d4ff);
                border_w = 2;
                rad = 15;
                break;
            
            default:
                bg = lv_color_hex(0x000000);
                border = lv_color_hex(0xFFFFFF);
                border_w = 3;
                rad = 15;
        }
        
        create(parent, width, height, bg, border, border_w, rad);
        
        // Ajouter ombre pour style CARD
        if (style == STYLE_CARD) {
            lv_obj_set_style_shadow_width(panel, 30, 0);
            lv_obj_set_style_shadow_color(panel, lv_color_black(), 0);
            lv_obj_set_style_shadow_opa(panel, LV_OPA_40, 0);
        }
        
        // Opacite reduite pour STYLE_INFO
        if (style == STYLE_INFO) {
            lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
        }
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, 400, 300, STYLE_DEFAULT);
    }
    
    /**
     * @brief Configure flex layout
     */
    void setFlexLayout(lv_flex_flow_t flow, 
                      lv_flex_align_t main_place = LV_FLEX_ALIGN_START,
                      lv_flex_align_t cross_place = LV_FLEX_ALIGN_START,
                      lv_flex_align_t track_place = LV_FLEX_ALIGN_START) {
        if (panel) {
            lv_obj_set_flex_flow(panel, flow);
            lv_obj_set_flex_align(panel, main_place, cross_place, track_place);
        }
    }
    
    /**
     * @brief Configure padding
     */
    void setPadding(lv_coord_t pad_all) {
        if (panel) {
            lv_obj_set_style_pad_all(panel, pad_all, 0);
        }
    }
    
    /**
     * @brief Configure padding row (pour flex column)
     */
    void setPaddingRow(lv_coord_t pad) {
        if (panel) {
            lv_obj_set_style_pad_row(panel, pad, 0);
        }
    }
    
    /**
     * @brief Configure padding column (pour flex row)
     */
    void setPaddingColumn(lv_coord_t pad) {
        if (panel) {
            lv_obj_set_style_pad_column(panel, pad, 0);
        }
    }
    
    /**
     * @brief Active/desactive scroll
     */
    void setScrollable(bool scrollable) {
        if (panel) {
            if (scrollable) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
            } else {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
            }
        }
    }
    
    /**
     * @brief Rend le fond transparent
     */
    void setTransparent() {
        if (panel) {
            lv_obj_set_style_bg_opa(panel, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(panel, 0, 0);
        }
    }
    
    /**
     * @brief Obtient l'objet panel pour ajouter des enfants
     */
    lv_obj_t* getPanel() const {
        return panel;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Panel = conteneur statique
    }
};

#endif