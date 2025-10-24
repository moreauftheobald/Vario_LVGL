#ifndef UI_SCREEN_PRESTART_H
#define UI_SCREEN_PRESTART_H

#include "UIScreenFileTransfer.h"
#include "UIScreenSettings.h"

class UIScreenPrestart : public UIScreen {
private:
  UIWidgetScreenFrame* screen_frame;
  UIWidgetLabel* label_title;
  UIWidgetPanel* panel_buttons;
  UIWidgetButton* btn_file_transfer;
  UIWidgetButton* btn_settings;
  UIWidgetButton* btn_start;
  UIWidgetPanel* panel_content;
  UIWidgetInfoPanel* info_panel;
  lv_obj_t* main_frame;
  static UIScreenPrestart* instance;

  static void btnFileTransferCallback(lv_event_t* e) {
    if (instance) instance->onFileTransferClicked();
  }

  static void btnSettingsCallback(lv_event_t* e) {
    if (instance) instance->onSettingsClicked();
  }

  static void btnStartCallback(lv_event_t* e) {
    if (instance) instance->onStartClicked();
  }

  void onFileTransferClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] File transfer clicked");
#endif

    if (lvgl_port_lock(-1)) {
      // Cacher ecran actuel
      hide();

      // Creer et afficher ecran file transfer
      UIScreenFileTransfer* file_transfer_screen = new UIScreenFileTransfer();
      file_transfer_screen->create();
      file_transfer_screen->load();

      lvgl_port_unlock();

#ifdef DEBUG_MODE
      Serial.println("[PRESTART] File transfer screen shown");
#endif
    }
  }

  void onSettingsClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Settings clicked");
#endif

    if (lvgl_port_lock(-1)) {
      hide();

      UIScreenSettings* settings_screen = new UIScreenSettings();
      settings_screen->create();
      settings_screen->load();

      lvgl_port_unlock();

#ifdef DEBUG_MODE
      Serial.println("[PRESTART] Settings screen shown");
#endif
    }
  }

  void onStartClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Start clicked");
#endif
  }

  void createMainFrame() {
    screen_frame = new UIWidgetScreenFrame();
    screen_frame->createFrame(screen);
    container = screen_frame->getFrame();
    screen = screen_frame->getScreen();
  }

  void createTitle() {
    label_title = new UIWidgetLabel();
    label_title->create(screen_frame->getFrame(), VARIO_NAME, UIWidgetLabel::STYLE_TITLE);
    lv_obj_set_style_text_align(label_title->getContainer(), LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_title->getContainer(), LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_opa(label_title->getContainer(), LV_OPA_TRANSP, 0);
  }

  void createContentContainer() {
    panel_content = new UIWidgetPanel();
    panel_content->create(screen_frame->getFrame(), LV_PCT(100), LV_SIZE_CONTENT,
                          lv_color_hex(0x000000), lv_color_hex(0x000000), 0, 0);
    panel_content->setTransparent();
    panel_content->setFlexLayout(LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY,
                                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    panel_content->setPaddingColumn(30);
    lv_obj_align(panel_content->getContainer(), LV_ALIGN_CENTER, 0, 40);
  }

  void createButtonColumn() {
    const TextStrings* txt = get_text();
    panel_buttons = new UIWidgetPanel();
    panel_buttons->create(panel_content->getPanel(), 440, 475,
                          lv_color_hex(0x000000), lv_color_hex(0x000000), 0, 0);
    panel_buttons->setTransparent();
    panel_buttons->setFlexLayout(LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_SPACE_BETWEEN);
    panel_buttons->setPaddingRow(20);
    lv_obj_t* btn_container = panel_buttons->getPanel();

    btn_file_transfer = new UIWidgetButton();
    btn_file_transfer->create(btn_container, txt->file_transfer, LV_SYMBOL_USB,
                              lv_color_hex(0x007aff));
    lv_obj_set_size(btn_file_transfer->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_file_transfer->getContainer(),
                        btnFileTransferCallback, LV_EVENT_CLICKED, nullptr);

    btn_settings = new UIWidgetButton();
    btn_settings->create(btn_container, txt->settings, LV_SYMBOL_SETTINGS,
                         lv_color_hex(0xff9500));
    lv_obj_set_size(btn_settings->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_settings->getContainer(),
                        btnSettingsCallback, LV_EVENT_CLICKED, nullptr);

    btn_start = new UIWidgetButton();
    btn_start->create(btn_container, txt->start, LV_SYMBOL_PLAY,
                      lv_color_hex(0x34c759));
    lv_obj_set_size(btn_start->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_start->getContainer(),
                        btnStartCallback, LV_EVENT_CLICKED, nullptr);
  }

  void createInfoPanel() {
    const TextStrings* txt = get_text();
    char info_text[128];

    info_panel = new UIWidgetInfoPanel();
    info_panel->create(panel_content->getPanel(), txt->information, 525, 475);

    // Carte SD avec espace libre
    if (sd_is_ready()) {
      uint64_t total_space = sd_get_total_bytes();
      uint64_t free_space = sd_get_free_bytes();
      snprintf(info_text, sizeof(info_text), "%s %s: %.1f/%.1f GB",
               LV_SYMBOL_SD_CARD, txt->sd_card,
               free_space / 1073741824.0, total_space / 1073741824.0);
    } else {
      snprintf(info_text, sizeof(info_text), "%s %s: --",
               LV_SYMBOL_SD_CARD, txt->sd_card);
    }
    info_panel->addInfoLine(info_text);

    // Cartes (Maps) - TODO: implementer compteur
    if (sd_is_ready()) {
      snprintf(info_text, sizeof(info_text), "%s %s: --",
               LV_SYMBOL_IMAGE, txt->maps);
    } else {
      snprintf(info_text, sizeof(info_text), "%s %s: --",
               LV_SYMBOL_IMAGE, txt->maps);
    }
    info_panel->addInfoLine(info_text);

    // Vols IGC
    if (sd_is_ready()) {
      int igc_count = sd_count_igc_files();
      snprintf(info_text, sizeof(info_text), "%s %s: %d",
               LV_SYMBOL_LIST, txt->flights, igc_count);
    } else {
      snprintf(info_text, sizeof(info_text), "%s %s: --",
               LV_SYMBOL_LIST, txt->flights);
    }
    info_panel->addInfoLine(info_text);

    // Pilote avec prenom + nom
    String pilot_display = String(LV_SYMBOL_HOME) + " " + txt->pilot + ": ";
    if (params.pilot_firstname != "" || params.pilot_name != "") {
      pilot_display += params.pilot_firstname;
      if (params.pilot_firstname != "" && params.pilot_name != "") {
        pilot_display += " ";
      }
      pilot_display += params.pilot_name;
    } else {
      pilot_display += "--";
    }
    info_panel->addInfoLine(pilot_display.c_str());

    // Telephone si renseigne
    if (params.pilot_phone != "") {
      String phone_display = String(LV_SYMBOL_CALL) + " " + txt->phone + ": " + params.pilot_phone;
      info_panel->addInfoLine(phone_display.c_str());
    }
  }

public:
  UIScreenPrestart()
    : UIScreen(), label_title(nullptr), panel_buttons(nullptr),
      btn_file_transfer(nullptr), btn_settings(nullptr), btn_start(nullptr),
      panel_content(nullptr), info_panel(nullptr), main_frame(nullptr) {}

  ~UIScreenPrestart() {
    if (label_title) delete label_title;
    if (panel_buttons) delete panel_buttons;
    if (btn_file_transfer) delete btn_file_transfer;
    if (btn_settings) delete btn_settings;
    if (btn_start) delete btn_start;
    if (panel_content) delete panel_content;
    if (info_panel) delete info_panel;
    if (instance == this) instance = nullptr;
  }

  void create(lv_obj_t* parent = nullptr) override {
    createMainFrame();
    createTitle();
    createContentContainer();
    createButtonColumn();
    createInfoPanel();
    initialized = true;
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Screen created with UIWidgetPanel containers");
#endif
  }

  void update() override {}

  void load() override {
    instance = this;
    UIScreen::load();
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Screen loaded");
#endif
  }

  void hide() override {
    if (instance == this) instance = nullptr;
    UIScreen::hide();
  }
};

#endif