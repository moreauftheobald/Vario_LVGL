#ifndef UI_SCREEN_SPLASH_H
#define UI_SCREEN_SPLASH_H

// Forward declaration pour le logo
LV_IMG_DECLARE(logo_bipbiphourra);

/**
 * @brief Ecran de demarrage (splash screen)
 * Affiche le logo pendant 3 secondes puis passe a l'ecran suivant
 */
class UIScreenSplash : public UIScreen {
private:
    lv_obj_t* logo_img;
    lv_obj_t* label_version;
    lv_obj_t* label_loading;
    lv_timer_t* timer;
    
    // Callback de transition externe
    void (*on_complete_callback)();
    
    // Pointeur statique pour callback (car lv_timer_t est incomplet en LVGL 9)
    static UIScreenSplash* instance;
    
    // Callback timer statique
    static void timerCallback(lv_timer_t* timer) {
        if (instance) {
            instance->onTimerExpired();
        }
    }
    
    // Gestion expiration timer
    void onTimerExpired() {
#ifdef DEBUG_MODE
        Serial.println("[SPLASH] Timer expired - transitioning to next screen");
#endif
        // Arreter le timer
        if (timer) {
            lv_timer_del(timer);
            timer = nullptr;
        }
        
        // Appeler callback si defini
        if (on_complete_callback) {
            on_complete_callback();
        }
        
        // Nettoyer instance
        instance = nullptr;
    }
    
public:
    UIScreenSplash() : UIScreen(), logo_img(nullptr), 
                       label_version(nullptr), label_loading(nullptr), 
                       timer(nullptr), on_complete_callback(nullptr) {}
    
    ~UIScreenSplash() {
        if (timer) {
            lv_timer_del(timer);
            timer = nullptr;
        }
    }
    
    /**
     * @brief Definit le callback appele a la fin du splash
     * @param callback Fonction a appeler pour transition
     */
    void setOnCompleteCallback(void (*callback)()) {
        on_complete_callback = callback;
    }
    
    void create(lv_obj_t* parent = nullptr) override {
        // Creer ecran avec fond beige clair
        screen = lv_obj_create(parent);
        container = screen;
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFF7E6), 0);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        
        // Logo centre
        logo_img = lv_image_create(screen);
        lv_image_set_src(logo_img, &logo_bipbiphourra);
        lv_obj_align(logo_img, LV_ALIGN_CENTER, 0, -40);
        
        // Version en bas
        label_version = lv_label_create(screen);
        lv_label_set_text_fmt(label_version, "Version %s", VARIO_VERSION);
        lv_obj_set_style_text_font(label_version, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(label_version, lv_color_hex(0x808080), 0);
        lv_obj_align(label_version, LV_ALIGN_BOTTOM_MID, 0, -80);
        
        // Message loading
        label_loading = lv_label_create(screen);
        lv_label_set_text(label_loading, "Initialisation...");
        lv_obj_set_style_text_font(label_loading, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label_loading, lv_color_hex(0x606060), 0);
        lv_obj_align(label_loading, LV_ALIGN_BOTTOM_MID, 0, -50);
        
        // Creer arc anime (loading indicator)
        lv_obj_t* arc = lv_arc_create(screen);
        lv_obj_set_size(arc, 50, 50);
        lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_arc_set_value(arc, 0);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0x00d4ff), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(arc, 6, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0xd0d0d0), LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 6, LV_PART_MAIN);
        
        // Animation de l'arc (0 -> 100 en 3 secondes)
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, arc);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_arc_set_value);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_set_duration(&a, 3000);
        lv_anim_start(&a);
        
        initialized = true;
        
#ifdef DEBUG_MODE
        Serial.println("[SPLASH] Screen created");
#endif
    }
    
    void update() override {
        // Pas d'update necessaire pour le splash
        // Le timer gere la transition
    }
    
    void load() override {
        UIScreen::load();
        
        // Enregistrer instance pour callback
        instance = this;
        
        // Demarrer le timer de 3 secondes
        timer = lv_timer_create(timerCallback, 3000, nullptr);
        lv_timer_set_repeat_count(timer, 1); // Une seule fois
        
#ifdef DEBUG_MODE
        Serial.println("[SPLASH] Screen loaded, timer started (3s)");
        Serial.println("[SPLASH] Will transition to prestart when timer expires");
#endif
    }
    
    void hide() override {
        // Arreter le timer si on cache l'ecran
        if (timer) {
            lv_timer_del(timer);
            timer = nullptr;
        }
        
        // Nettoyer instance
        if (instance == this) {
            instance = nullptr;
        }
        
        UIScreen::hide();
    }
};

// Initialisation membre statique
UIScreenSplash* UIScreenSplash::instance = nullptr;

#endif