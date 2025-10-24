#ifndef UI_SCREEN_SETTINGS_H
#define UI_SCREEN_SETTINGS_H

#include "UIScreenPrestart.h"

/**
 * @brief Ecran des parametres
 * Affiche 6 boutons pour acceder aux differents sous-menus de parametres
 */
class UIScreenSettings : public UIScreen {
private:
  UIWidgetScreenFrame* screen_frame;
  UIWidgetLabel* label_title;
  UIWidgetPanel* buttons_panel;
  UIWidgetButton* btn_pilot;
  UIWidgetButton* btn_wifi;
  UIWidgetButton* btn_screen;
  UIWidgetButton* btn_vario;
  UIWidgetButton* btn_map;
  UIWidgetButton* btn_system;
  UIWidgetButton* btn_back;
  
  static UIScreenSettings* instance;

  // ========================================================================
  // CALLBACKS
  // ========================================================================

  static void btnPilotCallback(lv_event_t* e) {
    if (instance) instance->onPilotClicked();
  }

  static void btnWifiCallback(lv_event_t* e) {
    if (instance) instance->onWifiClicked();
  }

  static void btnScreenCallback(lv_event_t* e) {
    if (instance) instance->onScreenClicked();
  }

  static void btnVarioCallback(lv_event_t* e) {
    if (instance) instance->onVarioClicked();
  }

  static void btnMapCallback(lv_event_t* e) {
    if (instance) instance->onMapClicked();
  }

  static void btnSystemCallback(lv_event_t* e) {
    if (instance) instance->onSystemClicked();
  }

  static void btnBackCallback(lv_event_t* e) {
    if (instance) instance->onBackClicked();
  }

  void onPilotClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Pilot settings clicked");
#endif
    // TODO: Implementer UIScreenSettingsPilot
  }

  void onWifiClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] WiFi settings clicked");
#endif
    // TODO: Implementer UIScreenSettingsWifi
  }

  void onScreenClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Screen calibration clicked");
#endif
    // TODO: Implementer UIScreenSettingsScreen
  }

  void onVarioClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Vario settings clicked");
#endif
    // TODO: Implementer UIScreenSettingsVario
  }

  void onMapClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Map settings clicked");
#endif
    // TODO: Implementer UIScreenSettingsMap
  }

  void onSystemClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] System settings clicked");
#endif
    // TODO: Implementer UIScreenSettingsSystem
  }

  void onBackClicked() {
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Back clicked");
#endif
    
    if (lvgl_port_lock(-1)) {
      hide();
      
      extern UIScreenPrestart* prestart_screen;
      if (prestart_screen) {
        prestart_screen->load();
      }
      
      lvgl_port_unlock();
    }
  }

  // ========================================================================
  // CREATION ECRAN
  // ========================================================================

  void createMainFrame() {
    screen_frame = new UIWidgetScreenFrame();
    screen_frame->createFrame(screen);
    container = screen_frame->getFrame();
    screen = screen_frame->getScreen();
  }

  void createTitle() {
    const TextStrings* txt = get_text();
    
    label_title = new UIWidgetLabel();
    label_title->create(screen_frame->getFrame(), txt->settings,
                       UIWidgetLabel::STYLE_TITLE);
    lv_obj_set_style_text_align(label_title->getContainer(), LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_title->getContainer(), LV_ALIGN_TOP_MID, 0, 0);
  }

  void createButtonsPanel() {
    const TextStrings* txt = get_text();
    
    // Panel pour les 6 boutons en 2 colonnes
    buttons_panel = new UIWidgetPanel();
    buttons_panel->create(screen_frame->getFrame(), 940, 450,
                         lv_color_hex(0x000000), lv_color_hex(0x000000), 0, 0);
    buttons_panel->setTransparent();
    buttons_panel->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, LV_FLEX_ALIGN_SPACE_EVENLY,
                                LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    buttons_panel->setPaddingColumn(20);
    buttons_panel->setPaddingRow(15);
    lv_obj_align(buttons_panel->getContainer(), LV_ALIGN_CENTER, 0, 15);
    
    lv_obj_t* panel = buttons_panel->getPanel();

    // Bouton Pilot Settings
    btn_pilot = new UIWidgetButton();
    btn_pilot->create(panel, txt->pilot_settings, LV_SYMBOL_HOME,
                     lv_color_hex(0x5856d6));
    lv_obj_set_size(btn_pilot->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_pilot->getContainer(),
                       btnPilotCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton WiFi Settings
    btn_wifi = new UIWidgetButton();
    btn_wifi->create(panel, txt->wifi_settings, LV_SYMBOL_WIFI,
                    lv_color_hex(0x007aff));
    lv_obj_set_size(btn_wifi->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_wifi->getContainer(),
                       btnWifiCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton Screen Calibration
    btn_screen = new UIWidgetButton();
    btn_screen->create(panel, txt->screen_calibration, LV_SYMBOL_SETTINGS,
                      lv_color_hex(0xff9500));
    lv_obj_set_size(btn_screen->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_screen->getContainer(),
                       btnScreenCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton Vario Settings
    btn_vario = new UIWidgetButton();
    btn_vario->create(panel, txt->vario_settings, LV_SYMBOL_UP,
                     lv_color_hex(0x4cd964));
    lv_obj_set_size(btn_vario->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_vario->getContainer(),
                       btnVarioCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton Map Settings
    btn_map = new UIWidgetButton();
    btn_map->create(panel, txt->map_settings, LV_SYMBOL_GPS,
                   lv_color_hex(0x34c759));
    lv_obj_set_size(btn_map->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_map->getContainer(),
                       btnMapCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton System Settings
    btn_system = new UIWidgetButton();
    btn_system->create(panel, txt->system_settings, LV_SYMBOL_LIST,
                      lv_color_hex(0x8e8e93));
    lv_obj_set_size(btn_system->getContainer(), 460, 80);
    lv_obj_add_event_cb(btn_system->getContainer(),
                       btnSystemCallback, LV_EVENT_CLICKED, nullptr);
  }

  void createBackButton() {
    const TextStrings* txt = get_text();
    
    btn_back = new UIWidgetButton();
    btn_back->create(screen_frame->getFrame(), txt->back, nullptr,
                    lv_color_hex(0xff3b30));
    lv_obj_set_size(btn_back->getContainer(), 300, 70);
    lv_obj_align(btn_back->getContainer(), LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_add_event_cb(btn_back->getContainer(),
                       btnBackCallback, LV_EVENT_CLICKED, nullptr);
  }

public:
  UIScreenSettings()
    : UIScreen(), screen_frame(nullptr), label_title(nullptr),
      buttons_panel(nullptr), btn_pilot(nullptr), btn_wifi(nullptr),
      btn_screen(nullptr), btn_vario(nullptr), btn_map(nullptr),
      btn_system(nullptr), btn_back(nullptr) {}

  ~UIScreenSettings() {
    if (label_title) delete label_title;
    if (buttons_panel) delete buttons_panel;
    if (btn_pilot) delete btn_pilot;
    if (btn_wifi) delete btn_wifi;
    if (btn_screen) delete btn_screen;
    if (btn_vario) delete btn_vario;
    if (btn_map) delete btn_map;
    if (btn_system) delete btn_system;
    if (btn_back) delete btn_back;
    if (screen_frame) delete screen_frame;
    if (instance == this) instance = nullptr;
  }

  void create(lv_obj_t* parent = nullptr) override {
    createMainFrame();
    createTitle();
    createButtonsPanel();
    createBackButton();
    initialized = true;
    
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Screen created");
#endif
  }

  void update() override {}

  void load() override {
    instance = this;
    UIScreen::load();
    
#ifdef DEBUG_MODE
    Serial.println("[SETTINGS] Screen loaded");
#endif
  }

  void hide() override {
    if (instance == this) instance = nullptr;
    UIScreen::hide();
  }
};

UIScreenSettings* UIScreenSettings::instance = nullptr;

#endif