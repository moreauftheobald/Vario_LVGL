#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>

class UIManager {
private:
    static UIManager* instance;
    std::vector<UIScreen*> screens;
    uint8_t current_screen_idx;
    TaskHandle_t swipe_task_handle;
    
    // Swipe detection
    lv_point_t touch_start;
    bool touch_started;
    
    static void swipeEventHandler(lv_event_t* e) {
        UIManager::getInstance()->handleSwipeEvent(e);
    }
    
    void handleSwipeEvent(lv_event_t* e) {
        lv_event_code_t code = lv_event_get_code(e);
        
        if (code == LV_EVENT_PRESSED) {
            lv_indev_t* indev = lv_indev_active();
            if (indev) {
                lv_indev_get_point(indev, &touch_start);
                touch_started = true;
            }
        }
        else if (code == LV_EVENT_RELEASED && touch_started) {
            lv_indev_t* indev = lv_indev_active();
            if (indev) {
                lv_point_t touch_end;
                lv_indev_get_point(indev, &touch_end);
                
                int32_t diff_x = touch_end.x - touch_start.x;
                
                if (abs(diff_x) > 80) {
                    if (diff_x > 0 && current_screen_idx > 0) {
                        switchScreen(current_screen_idx - 1);
                    } else if (diff_x < 0 && 
                              current_screen_idx < screens.size() - 1) {
                        switchScreen(current_screen_idx + 1);
                    }
                }
            }
            touch_started = false;
        }
    }
    
    UIManager() : current_screen_idx(0), 
                  swipe_task_handle(nullptr), 
                  touch_started(false) {}
    
public:
    static UIManager* getInstance() {
        if (!instance) {
            instance = new UIManager();
        }
        return instance;
    }
    
    void init() {
        // Creer les ecrans
        screens.push_back(new UIScreenStats());
        screens.push_back(new UIScreenMain());
        screens.push_back(new UIScreenMap());
        
        // Initialiser tous les ecrans
        for (auto screen : screens) {
            screen->create();
            screen->hide();
        }
        
        // Charger ecran principal
        switchScreen(1);
    }
    
    void switchScreen(uint8_t idx) {
        if (idx >= screens.size()) return;
        
        // Cacher ecran actuel
        if (current_screen_idx < screens.size()) {
            screens[current_screen_idx]->hide();
        }
        
        // Afficher nouvel ecran
        current_screen_idx = idx;
        screens[current_screen_idx]->load();
        
        // Attacher swipe handler
        lv_obj_add_event_cb(screens[current_screen_idx]->getContainer(),
                           swipeEventHandler, 
                           LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(screens[current_screen_idx]->getContainer(),
                           swipeEventHandler, 
                           LV_EVENT_RELEASED, nullptr);
        
#ifdef DEBUG_MODE
        Serial.printf("[UI] Switch to screen %d\n", idx);
#endif
    }
    
    UIScreen* getCurrentScreen() {
        return screens[current_screen_idx];
    }
    
    ~UIManager() {
        for (auto screen : screens) {
            delete screen;
        }
        screens.clear();
    }
};

UIManager* UIManager::instance = nullptr;

#endif