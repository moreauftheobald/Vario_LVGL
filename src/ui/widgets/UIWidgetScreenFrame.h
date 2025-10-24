#ifndef UI_WIDGET_SCREEN_FRAME_H
#define UI_WIDGET_SCREEN_FRAME_H

#include "../UIWidget.h"

/**
 * @brief Widget frame d'ecran standard
 * Cree un ecran noir avec frame bordure blanche
 * Utilise par tous les ecrans de l'application
 */
class UIWidgetScreenFrame : public UIWidget {
private:
    lv_obj_t* screen;
    lv_obj_t* frame;
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetScreenFrame(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          screen(nullptr),
          frame(nullptr) {}
    
    ~UIWidgetScreenFrame() {
        // Cleanup auto
    }
    
    /**
     * @brief Cree le frame d'ecran standard
     * @param screen_obj Objet screen existant (ou nullptr pour en creer un)
     * @param border_width Largeur bordure (defaut: 3)
     * @param border_color Couleur bordure (defaut: blanc)
     * @param radius Rayon coins (defaut: 20)
     */
    void createFrame(lv_obj_t* screen_obj = nullptr, 
                     int border_width = 3,
                     lv_color_t border_color = lv_color_hex(0xFFFFFF),
                     int radius = 20) {
        
        // Ecran noir
        if (screen_obj) {
            screen = screen_obj;
            lv_obj_clean(screen);
        } else {
            screen = lv_obj_create(nullptr);
        }
        
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(screen, 0, 0);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        
        // Frame avec bordure
        frame = lv_obj_create(screen);
        container = frame;
        
        lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES);
        lv_obj_center(frame);
        
        // Style frame
        lv_obj_set_style_bg_color(frame, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(frame, border_width, 0);
        lv_obj_set_style_border_color(frame, border_color, 0);
        lv_obj_set_style_radius(frame, radius, 0);
        lv_obj_set_style_pad_all(frame, 20, 0);
        
        // Ombre
        lv_obj_set_style_shadow_width(frame, 30, 0);
        lv_obj_set_style_shadow_color(frame, lv_color_black(), 0);
        lv_obj_set_style_shadow_opa(frame, LV_OPA_40, 0);
        
        lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        createFrame(nullptr, 3, lv_color_hex(0xFFFFFF), 20);
    }
    
    /**
     * @brief Obtient l'objet screen
     */
    lv_obj_t* getScreen() const {
        return screen;
    }
    
    /**
     * @brief Obtient l'objet frame (conteneur principal)
     */
    lv_obj_t* getFrame() const {
        return frame;
    }
    
    /**
     * @brief Change la couleur de bordure
     */
    void setBorderColor(lv_color_t color) {
        if (frame) {
            lv_obj_set_style_border_color(frame, color, 0);
        }
    }
    
    /**
     * @brief Change la largeur de bordure
     */
    void setBorderWidth(int width) {
        if (frame) {
            lv_obj_set_style_border_width(frame, width, 0);
        }
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Widget statique
    }
};

#endif