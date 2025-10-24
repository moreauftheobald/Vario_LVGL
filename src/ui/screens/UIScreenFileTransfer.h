#ifndef UI_SCREEN_FILE_TRANSFER_H
#define UI_SCREEN_FILE_TRANSFER_H

#include "../UIScreen.h"
#include "../widgets/UIWidgetScreenFrame.h"
#include "../widgets/UIWidgetLabel.h"
#include "../widgets/UIWidgetPanel.h"
#include "../widgets/UIWidgetSeparator.h"
#include "../widgets/UIWidgetButton.h"
#include "constants.h"
#include "globals.h"
#include "lang.h"
#include "esp_system.h"
#include "src/wifi_task.h"
#include "src/file_server_task.h"

/**
 * @brief Ecran de transfert de fichiers via WiFi
 * Affiche statut WiFi, SSID, IP et instructions
 * Demarre automatiquement WiFi + serveur fichiers
 */
class UIScreenFileTransfer : public UIScreen {
private:
    // Widgets
    UIWidgetScreenFrame* screen_frame;
    UIWidgetLabel* label_title;
    UIWidgetPanel* info_panel;
    UIWidgetLabel* label_status;
    UIWidgetLabel* label_ssid;
    UIWidgetLabel* label_ip;
    UIWidgetSeparator* separator;
    UIWidgetLabel* label_instructions;
    UIWidgetButton* btn_exit;
    
    // Timer LVGL pour MAJ statut
    lv_timer_t* status_timer;
    
    // Instance statique
    static UIScreenFileTransfer* instance;
    
    // ========================================================================
    // TIMER ET CALLBACKS
    // ========================================================================
    
    /**
     * @brief Callback timer statique pour LVGL
     */
    static void statusTimerCallback(lv_timer_t* timer) {
        if (instance) {
            instance->updateStatus();
        }
    }
    
    /**
     * @brief Met a jour le statut WiFi
     */
    void updateStatus() {
        if (!label_status || !label_ssid || !label_ip) return;
        
        if (wifi_get_connected_status()) {
            // WiFi connecte
            label_status->setText(LV_SYMBOL_WIFI " Connected");
            label_status->setColor(lv_color_hex(0x34c759)); // Vert
            
            // SSID
            char ssid_buf[64];
            snprintf(ssid_buf, sizeof(ssid_buf), "SSID: %s", wifi_get_current_ssid());
            label_ssid->setText(ssid_buf);
            
            // IP
            char ip_buf[32];
            snprintf(ip_buf, sizeof(ip_buf), "IP: %s", wifi_get_current_ip());
            label_ip->setText(ip_buf);
        } else {
            // WiFi deconnecte
            label_status->setText(LV_SYMBOL_WARNING " Connecting...");
            label_status->setColor(lv_color_hex(0xff9500)); // Orange
            label_ssid->setText("Trying WiFi networks...");
            label_ip->setText("");
        }
    }
    
    /**
     * @brief Callback bouton Exit
     */
    static void btnExitCallback(lv_event_t* e) {
        if (instance) {
            instance->onExitClicked();
        }
    }
    
    void onExitClicked() {
#ifdef DEBUG_MODE
        Serial.println("[FILE_TRANSFER] Exit clicked - stopping services");
#endif
        
        // Arreter timer
        if (status_timer) {
            lv_timer_del(status_timer);
            status_timer = nullptr;
        }
        
        // Arreter services
        file_server_stop();
        wifi_task_stop();
        
        // Attendre un peu
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Redemarrer
        esp_restart();
    }
    
    // ========================================================================
    // CREATION ECRAN
    // ========================================================================
    
    void createMainFrame() {
        screen_frame = new UIWidgetScreenFrame();
        screen_frame->createFrame(screen);
        screen = screen_frame->getScreen();
        container = screen;
    }
    
    void createTitle() {
        const TextStrings* txt = get_text();
        
        label_title = new UIWidgetLabel();
        label_title->create(screen_frame->getFrame(), txt->file_transfer,
                          UIWidgetLabel::STYLE_SUBTITLE);
        lv_obj_align(label_title->getContainer(), LV_ALIGN_TOP_MID, 0, 0);
    }
    
    void createInfoPanel() {
        const TextStrings* txt = get_text();
        
        // Panel principal
        info_panel = new UIWidgetPanel();
        info_panel->create(screen_frame->getFrame(), 900, 400,
                         lv_color_hex(0x1a2035), lv_color_hex(0x6080a0), 2, 15);
        info_panel->setFlexLayout(LV_FLEX_FLOW_COLUMN,
                                 LV_FLEX_ALIGN_START,
                                 LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_CENTER);
        info_panel->setPaddingRow(20);
        lv_obj_set_style_bg_opa(info_panel->getPanel(), LV_OPA_80, 0);
        lv_obj_align(info_panel->getContainer(), LV_ALIGN_CENTER, 0, 10);
        
        lv_obj_t* panel = info_panel->getPanel();
        
        // Status WiFi (gros)
        label_status = new UIWidgetLabel();
        label_status->create(panel, LV_SYMBOL_WIFI " Starting...",
                           &lv_font_montserrat_32, lv_color_hex(0xff9500));
        
        // SSID
        label_ssid = new UIWidgetLabel();
        label_ssid->create(panel, "Initializing WiFi...",
                         UIWidgetLabel::STYLE_INFO);
        lv_obj_set_style_text_align(label_ssid->getContainer(),
                                   LV_TEXT_ALIGN_CENTER, 0);
        
        // IP
        label_ip = new UIWidgetLabel();
        label_ip->create(panel, "", UIWidgetLabel::STYLE_INFO);
        lv_obj_set_style_text_align(label_ip->getContainer(),
                                   LV_TEXT_ALIGN_CENTER, 0);
        
        // Separateur
        separator = new UIWidgetSeparator();
        separator->create(panel, UIWidgetSeparator::HORIZONTAL,
                        UIWidgetSeparator::STYLE_MEDIUM);
        
        // Instructions
        label_instructions = new UIWidgetLabel();
        label_instructions->create(panel,
                                  "1. Wait for WiFi connection\n"
                                  "2. Open your web browser\n"
                                  "3. Go to the IP address shown above",
                                  UIWidgetLabel::STYLE_INFO);
        lv_obj_set_style_text_align(label_instructions->getContainer(),
                                   LV_TEXT_ALIGN_CENTER, 0);
        label_instructions->setLongMode(LV_LABEL_LONG_WRAP);
        label_instructions->setWidth(LV_PCT(90));
    }
    
    void createExitButton() {
        const TextStrings* txt = get_text();
        
        btn_exit = new UIWidgetButton();
        btn_exit->create(screen_frame->getFrame(), txt->exit, nullptr,
                       lv_color_hex(0xff3b30)); // Rouge
        lv_obj_set_size(btn_exit->getContainer(), 300, 70);
        lv_obj_align(btn_exit->getContainer(), LV_ALIGN_BOTTOM_MID, 0, -15);
        lv_obj_add_event_cb(btn_exit->getContainer(),
                          btnExitCallback, LV_EVENT_CLICKED, nullptr);
    }
    
    void startServices() {
#ifdef DEBUG_MODE
        Serial.println("[FILE_TRANSFER] Starting WiFi and file server");
#endif
        
        // Demarrer WiFi
        wifi_task_start();
        
        // Attendre un peu
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Demarrer serveur fichiers
        file_server_start();
        
        // Creer timer MAJ statut (1 seconde)
        status_timer = lv_timer_create(statusTimerCallback, 1000, nullptr);
        
#ifdef DEBUG_MODE
        Serial.println("[FILE_TRANSFER] Services started");
#endif
    }
    
public:
    UIScreenFileTransfer() : UIScreen(),
                            screen_frame(nullptr),
                            label_title(nullptr),
                            info_panel(nullptr),
                            label_status(nullptr),
                            label_ssid(nullptr),
                            label_ip(nullptr),
                            separator(nullptr),
                            label_instructions(nullptr),
                            btn_exit(nullptr),
                            status_timer(nullptr) {}
    
    ~UIScreenFileTransfer() {
        // Arreter timer
        if (status_timer) {
            lv_timer_del(status_timer);
            status_timer = nullptr;
        }
        
        // Supprimer widgets
        if (screen_frame) delete screen_frame;
        if (label_title) delete label_title;
        if (info_panel) delete info_panel;
        if (label_status) delete label_status;
        if (label_ssid) delete label_ssid;
        if (label_ip) delete label_ip;
        if (separator) delete separator;
        if (label_instructions) delete label_instructions;
        if (btn_exit) delete btn_exit;
        
        if (instance == this) instance = nullptr;
    }
    
    void create(lv_obj_t* parent = nullptr) override {
        createMainFrame();
        createTitle();
        createInfoPanel();
        createExitButton();
        
        initialized = true;
        
#ifdef DEBUG_MODE
        Serial.println("[FILE_TRANSFER] Screen created");
#endif
    }
    
    void update() override {
        // Pas de MAJ manuelle, le timer gere tout
    }
    
    void load() override {
        instance = this;
        UIScreen::load();
        
        // Demarrer services WiFi + serveur
        startServices();
        
#ifdef DEBUG_MODE
        Serial.println("[FILE_TRANSFER] Screen loaded");
#endif
    }
    
    void hide() override {
        // Arreter timer si on quitte l'ecran
        if (status_timer) {
            lv_timer_del(status_timer);
            status_timer = nullptr;
        }
        
        if (instance == this) instance = nullptr;
        UIScreen::hide();
    }
};

// Initialisation membre statique
UIScreenFileTransfer* UIScreenFileTransfer::instance = nullptr;

#endif