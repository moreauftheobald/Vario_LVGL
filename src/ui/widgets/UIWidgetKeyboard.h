#ifndef UI_WIDGET_KEYBOARD_H
#define UI_WIDGET_KEYBOARD_H

/**
 * @brief Widget clavier virtuel global
 * Singleton partage entre tous les TextArea de l'application
 * Gere automatiquement l'affichage/masquage
 */
class UIWidgetKeyboard : public UIWidget {
private:
    lv_obj_t* keyboard;
    lv_obj_t* current_textarea;
    
    // Singleton
    static UIWidgetKeyboard* instance;
    
    // Callbacks externes
    void (*on_ready_callback)(void* user_data);
    void (*on_cancel_callback)(void* user_data);
    void* user_data;
    
    // Callback statique pour LVGL
    static void keyboardEventCallback(lv_event_t* e) {
        UIWidgetKeyboard* self = (UIWidgetKeyboard*)lv_event_get_user_data(e);
        if (!self) return;
        
        lv_event_code_t code = lv_event_get_code(e);
        
        if (code == LV_EVENT_READY) {
            // Valider et cacher clavier
            self->hide();
            
            if (self->on_ready_callback) {
                self->on_ready_callback(self->user_data);
            }
            
#ifdef DEBUG_MODE
            Serial.println("[KEYBOARD] Ready - text validated");
#endif
        }
        else if (code == LV_EVENT_CANCEL) {
            // Annuler et cacher clavier
            self->hide();
            
            if (self->on_cancel_callback) {
                self->on_cancel_callback(self->user_data);
            }
            
#ifdef DEBUG_MODE
            Serial.println("[KEYBOARD] Cancel - text discarded");
#endif
        }
    }
    
    // Constructeur prive (singleton)
    UIWidgetKeyboard(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          keyboard(nullptr),
          current_textarea(nullptr),
          on_ready_callback(nullptr),
          on_cancel_callback(nullptr),
          user_data(nullptr) {}
    
public:
    /**
     * @brief Modes de clavier
     */
    enum Mode {
        MODE_TEXT_LOWER,    // Texte minuscules
        MODE_TEXT_UPPER,    // Texte majuscules
        MODE_SPECIAL,       // Caracteres speciaux
        MODE_NUMBER         // Numerique
    };
    
    /**
     * @brief Obtient l'instance singleton
     */
    static UIWidgetKeyboard* getInstance() {
        if (!instance) {
            instance = new UIWidgetKeyboard();
        }
        return instance;
    }
    
    /**
     * @brief Supprime l'instance (appeler a la fin de l'application)
     */
    static void destroyInstance() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }
    
    ~UIWidgetKeyboard() {
        if (instance == this) {
            instance = nullptr;
        }
    }
    
    /**
     * @brief Cree le clavier (a appeler UNE SEULE FOIS au demarrage)
     * @param parent Parent LVGL (generalement l'ecran principal)
     * @param mode Mode initial du clavier
     */
    void create(lv_obj_t* parent, Mode mode = MODE_TEXT_LOWER) {
        if (keyboard) {
#ifdef DEBUG_MODE
            Serial.println("[KEYBOARD] Already created, skipping");
#endif
            return;
        }
        
        // Creer clavier
        keyboard = lv_keyboard_create(parent);
        container = keyboard;
        
        // Taille et position
        lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(40));
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        
        // Mode initial
        setMode(mode);
        
        // Cacher par defaut
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        
        // Callback evenements
        lv_obj_add_event_cb(keyboard, keyboardEventCallback, 
                          LV_EVENT_ALL, this);
        
        // Mettre au premier plan
        lv_obj_move_foreground(keyboard);
        
        initialized = true;
        
#ifdef DEBUG_MODE
        Serial.println("[KEYBOARD] Created and hidden");
#endif
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, MODE_TEXT_LOWER);
    }
    
    /**
     * @brief Attache le clavier a un textarea et l'affiche
     * @param textarea Textarea LVGL a editer
     */
    void attachTo(lv_obj_t* textarea) {
        if (!keyboard || !textarea) return;
        
        current_textarea = textarea;
        lv_keyboard_set_textarea(keyboard, textarea);
        show();
        
#ifdef DEBUG_MODE
        Serial.println("[KEYBOARD] Attached to textarea");
#endif
    }
    
    /**
     * @brief Detache le clavier et le cache
     */
    void detach() {
        if (!keyboard) return;
        
        lv_keyboard_set_textarea(keyboard, nullptr);
        current_textarea = nullptr;
        hide();
        
#ifdef DEBUG_MODE
        Serial.println("[KEYBOARD] Detached");
#endif
    }
    
    /**
     * @brief Affiche le clavier
     */
    void show() override {
        if (keyboard && !visible) {
            lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(keyboard);
            visible = true;
            
#ifdef DEBUG_MODE
            Serial.println("[KEYBOARD] Shown");
#endif
        }
    }
    
    /**
     * @brief Cache le clavier
     */
    void hide() override {
        if (keyboard && visible) {
            lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
            visible = false;
            
#ifdef DEBUG_MODE
            Serial.println("[KEYBOARD] Hidden");
#endif
        }
    }
    
    /**
     * @brief Change le mode du clavier
     */
    void setMode(Mode mode) {
        if (!keyboard) return;
        
        lv_keyboard_mode_t lv_mode;
        
        switch(mode) {
            case MODE_TEXT_LOWER:
                lv_mode = LV_KEYBOARD_MODE_TEXT_LOWER;
                break;
            
            case MODE_TEXT_UPPER:
                lv_mode = LV_KEYBOARD_MODE_TEXT_UPPER;
                break;
            
            case MODE_SPECIAL:
                lv_mode = LV_KEYBOARD_MODE_SPECIAL;
                break;
            
            case MODE_NUMBER:
                lv_mode = LV_KEYBOARD_MODE_NUMBER;
                break;
            
            default:
                lv_mode = LV_KEYBOARD_MODE_TEXT_LOWER;
        }
        
        lv_keyboard_set_mode(keyboard, lv_mode);
    }
    
    /**
     * @brief Definit callbacks Ready et Cancel
     * @param ready_cb Callback appele quand utilisateur valide (OK)
     * @param cancel_cb Callback appele quand utilisateur annule
     * @param data Donnees utilisateur passees aux callbacks
     */
    void setCallbacks(void (*ready_cb)(void*) = nullptr,
                     void (*cancel_cb)(void*) = nullptr,
                     void* data = nullptr) {
        on_ready_callback = ready_cb;
        on_cancel_callback = cancel_cb;
        user_data = data;
    }
    
    /**
     * @brief Verifie si le clavier est actuellement affiche
     */
    bool isShown() const {
        return visible;
    }
    
    /**
     * @brief Obtient le textarea actuellement attache
     */
    lv_obj_t* getCurrentTextArea() const {
        return current_textarea;
    }
    
    /**
     * @brief Obtient l'objet clavier LVGL brut
     */
    lv_obj_t* getKeyboard() const {
        return keyboard;
    }
    
    /**
     * @brief Pas de MAJ periodique
     */
    void update() override {
        // Clavier = widget statique
    }
};

// Initialisation statique
UIWidgetKeyboard* UIWidgetKeyboard::instance = nullptr;

#endif