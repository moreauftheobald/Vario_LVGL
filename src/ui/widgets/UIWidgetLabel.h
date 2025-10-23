#ifndef UI_WIDGET_LABEL_H
#define UI_WIDGET_LABEL_H

/**
 * @brief Widget label stylise avec presets de style
 * Support texte dynamique, couleurs, fonts
 */
class UIWidgetLabel : public UIWidget {
private:
    lv_obj_t* label;
    const lv_font_t* font;
    lv_color_t color;
    
public:
    /**
     * @brief Styles predéfinis
     */
    enum Style {
        STYLE_TITLE,        // Grand titre bleu
        STYLE_SUBTITLE,     // Sous-titre moyen
        STYLE_NORMAL,       // Texte normal blanc
        STYLE_INFO,         // Info gris clair
        STYLE_SUCCESS,      // Vert
        STYLE_WARNING,      // Orange
        STYLE_ERROR,        // Rouge
        STYLE_SMALL         // Petit texte
    };
    
    /**
     * @brief Constructeur
     * @param update_interval_ms Intervalle MAJ (0 = pas de MAJ auto)
     */
    UIWidgetLabel(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          label(nullptr),
          font(&lv_font_montserrat_20),
          color(lv_color_white()) {}
    
    ~UIWidgetLabel() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le label avec style personnalise
     */
    void create(lv_obj_t* parent, const char* text,
               const lv_font_t* label_font, lv_color_t label_color) {
        
        font = label_font;
        color = label_color;
        
        label = lv_label_create(parent);
        container = label;
        
        if (text) {
            lv_label_set_text(label, text);
        }
        
        lv_obj_set_style_text_font(label, font, 0);
        lv_obj_set_style_text_color(label, color, 0);
        
        initialized = true;
    }
    
    /**
     * @brief Cree le label avec style predefini
     */
    void create(lv_obj_t* parent, const char* text, Style style) {
        const lv_font_t* preset_font;
        lv_color_t preset_color;
        
        switch(style) {
            case STYLE_TITLE:
                preset_font = &lv_font_montserrat_48;
                preset_color = lv_color_hex(0x00d4ff);
                break;
            
            case STYLE_SUBTITLE:
                preset_font = &lv_font_montserrat_32;
                preset_color = lv_color_hex(0x00d4ff);
                break;
            
            case STYLE_NORMAL:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_white();
                break;
            
            case STYLE_INFO:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_hex(0xaabbcc);
                break;
            
            case STYLE_SUCCESS:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_hex(0x34c759);
                break;
            
            case STYLE_WARNING:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_hex(0xff9500);
                break;
            
            case STYLE_ERROR:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_hex(0xff3b30);
                break;
            
            case STYLE_SMALL:
                preset_font = &lv_font_montserrat_16;
                preset_color = lv_color_hex(0x808080);
                break;
            
            default:
                preset_font = &lv_font_montserrat_20;
                preset_color = lv_color_white();
        }
        
        create(parent, text, preset_font, preset_color);
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Label", STYLE_NORMAL);
    }
    
    /**
     * @brief Change le texte
     */
    void setText(const char* text) {
        if (label) {
            lv_label_set_text(label, text);
        }
    }
    
    /**
     * @brief Change le texte avec format printf
     */
    void setTextFmt(const char* fmt, ...) {
        if (label) {
            va_list args;
            va_start(args, fmt);
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            lv_label_set_text(label, buffer);
        }
    }
    
    /**
     * @brief Change la couleur
     */
    void setColor(lv_color_t new_color) {
        color = new_color;
        if (label) {
            lv_obj_set_style_text_color(label, color, 0);
        }
    }
    
    /**
     * @brief Change la font
     */
    void setFont(const lv_font_t* new_font) {
        font = new_font;
        if (label) {
            lv_obj_set_style_text_font(label, font, 0);
        }
    }
    
    /**
     * @brief Configure l'alignement du texte
     */
    void setAlign(lv_text_align_t align) {
        if (label) {
            lv_obj_set_style_text_align(label, align, 0);
        }
    }
    
    /**
     * @brief Configure le mode long texte
     */
    void setLongMode(lv_label_long_mode_t mode) {
        if (label) {
            lv_label_set_long_mode(label, mode);
        }
    }
    
    /**
     * @brief Definit la largeur (pour wrap)
     */
    void setWidth(lv_coord_t width) {
        if (label) {
            lv_obj_set_width(label, width);
        }
    }
    
    /**
     * @brief Pas de MAJ periodique par defaut
     */
    void update() override {
        // Label statique par defaut
    }
};

#endif