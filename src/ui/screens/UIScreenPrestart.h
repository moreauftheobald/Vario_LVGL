#ifndef UI_SCREEN_PRESTART_H
#define UI_SCREEN_PRESTART_H

/**
 * @brief Ecran pre-demarrage refactorise avec widgets
 * Affiche 3 boutons + panel d'informations systeme
 */
class UIScreenPrestart : public UIScreen {
private:
  // Widgets
  UIWidgetScreenFrame* screen_frame;
  UIWidgetLabel* label_title;
  UIWidgetPanel* panel_buttons;  // Panel pour les boutons
  UIWidgetButton* btn_file_transfer;
  UIWidgetButton* btn_settings;
  UIWidgetButton* btn_start;
  UIWidgetPanel* panel_content;  // Panel conteneur 2 colonnes
  UIWidgetInfoPanel* info_panel;

  // Conteneurs
  lv_obj_t* main_frame;

  // Instance statique pour callbacks
  static UIScreenPrestart* instance;

  // ========================================================================
  // CALLBACKS BOUTONS
  // ========================================================================

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
    //extern void ui_file_transfer_show(void);
    //ui_file_transfer_show();
  }

  void onSettingsClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Settings clicked");
#endif
    //extern void ui_settings_show(void);
    //ui_settings_show();
  }

  void onStartClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Start clicked");
#endif
    //extern void ui_main_screens_show(void);
    //ui_main_screens_show();
  }

  // ========================================================================
  // CREATION ECRAN
  // ========================================================================

  /**
     * @brief Cree le frame principal avec UIWidgetScreenFrame
     */
  void createMainFrame() {
    screen_frame = new UIWidgetScreenFrame();
    screen_frame->createFrame(screen);  // ← Plus d'ambiguïté !

    screen = screen_frame->getScreen();
    container = screen;
  }
  /**
     * @brief Cree le titre
     */
  void createTitle() {
    label_title = new UIWidgetLabel();
    label_title->create(screen_frame->getFrame(), VARIO_NAME, UIWidgetLabel::STYLE_TITLE);
    lv_obj_set_style_text_align(label_title->getContainer(), LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_title->getContainer(), LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_opa(label_title->getContainer(), LV_OPA_TRANSP, 0);
  }

  /**
     * @brief Cree le conteneur principal 2 colonnes avec UIWidgetPanel
     */
  void createContentContainer() {
    panel_content = new UIWidgetPanel();
    panel_content->create(screen_frame->getFrame(), LV_PCT(100), LV_SIZE_CONTENT,
                          lv_color_hex(0x000000), lv_color_hex(0x000000), 0, 0);
    panel_content->setTransparent();
    panel_content->setFlexLayout(LV_FLEX_FLOW_ROW,
                                 LV_FLEX_ALIGN_SPACE_EVENLY,
                                 LV_FLEX_ALIGN_CENTER,
                                 LV_FLEX_ALIGN_CENTER);
    panel_content->setPaddingColumn(30);

    lv_obj_align(panel_content->getContainer(), LV_ALIGN_CENTER, 0, 40);
  }

  /**
     * @brief Cree la colonne de boutons avec UIWidgetPanel
     */
  void createButtonColumn() {
    const TextStrings* txt = get_text();

    // Panel colonne boutons (transparent)
    panel_buttons = new UIWidgetPanel();
    panel_buttons->create(panel_content->getPanel(), 440, 475,
                          lv_color_hex(0x000000), lv_color_hex(0x000000), 0, 0);
    panel_buttons->setTransparent();
    panel_buttons->setFlexLayout(LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_SPACE_BETWEEN);
    panel_buttons->setPaddingRow(20);

    lv_obj_t* btn_container = panel_buttons->getPanel();

    // Bouton File Transfer
    btn_file_transfer = new UIWidgetButton();
    btn_file_transfer->create(btn_container, txt->file_transfer, LV_SYMBOL_USB,
                              lv_color_hex(0x007aff));
    lv_obj_set_size(btn_file_transfer->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_file_transfer->getContainer(),
                        btnFileTransferCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton Settings
    btn_settings = new UIWidgetButton();
    btn_settings->create(btn_container, txt->settings, LV_SYMBOL_SETTINGS,
                         lv_color_hex(0xff9500));
    lv_obj_set_size(btn_settings->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_settings->getContainer(),
                        btnSettingsCallback, LV_EVENT_CLICKED, nullptr);

    // Bouton Start
    btn_start = new UIWidgetButton();
    btn_start->create(btn_container, txt->start, LV_SYMBOL_PLAY,
                      lv_color_hex(0x34c759));
    lv_obj_set_size(btn_start->getContainer(), 400, 120);
    lv_obj_add_event_cb(btn_start->getContainer(),
                        btnStartCallback, LV_EVENT_CLICKED, nullptr);
  }

  /**
     * @brief Cree le panel d'informations
     */
  void createInfoPanel() {
    const TextStrings* txt = get_text();

    // Creer panel dans le conteneur principal
    info_panel = new UIWidgetInfoPanel();
    info_panel->create(panel_content->getPanel(), txt->information, 525, 475);

    // Version
    char buf[128];
    snprintf(buf, sizeof(buf), "%s: %s", txt->version, VARIO_VERSION);
    info_panel->addInfoLineWithIcon(LV_SYMBOL_SETTINGS, buf);

    // SD Card
    if (sd_is_ready()) {
      uint64_t total = sd_get_total_bytes();
      uint64_t used = sd_get_used_bytes();
      snprintf(buf, sizeof(buf), "%s: %.1f / %.1f MB",
               txt->sd_card,
               used / (1024.0 * 1024.0),
               total / (1024.0 * 1024.0));
    } else {
      snprintf(buf, sizeof(buf), "%s: Non disponible", txt->sd_card);
    }
    info_panel->addInfoLineWithIcon(LV_SYMBOL_SD_CARD, buf);

    // Maps (TODO: compter vraiment les tiles)
    snprintf(buf, sizeof(buf), "%s: --", txt->maps);
    info_panel->addInfoLineWithIcon(LV_SYMBOL_IMAGE, buf);

    // Vols IGC
    if (sd_is_ready()) {
      int count = sd_count_igc_files();
      snprintf(buf, sizeof(buf), "%s: %d", txt->flights, count);
    } else {
      snprintf(buf, sizeof(buf), "%s: --", txt->flights);
    }
    info_panel->addInfoLineWithIcon(LV_SYMBOL_LIST, buf);

    // Pilote
    String pilot_str = String(txt->pilot) + ": ";
    if (params.pilot_firstname != "" || params.pilot_name != "") {
      pilot_str += params.pilot_firstname;
      if (params.pilot_firstname != "" && params.pilot_name != "") {
        pilot_str += " ";
      }
      pilot_str += params.pilot_name;
    } else {
      pilot_str += "--";
    }
    info_panel->addInfoLineWithIcon(LV_SYMBOL_HOME, pilot_str.c_str());

    // Telephone (si renseigne)
    if (params.pilot_phone != "") {
      String phone_str = String(txt->phone) + ": " + params.pilot_phone;
      info_panel->addInfoLineWithIcon(LV_SYMBOL_CALL, phone_str.c_str());
    }
  }

public:
  UIScreenPrestart()
    : UIScreen(),
      label_title(nullptr),
      panel_buttons(nullptr),
      btn_file_transfer(nullptr),
      btn_settings(nullptr),
      btn_start(nullptr),
      panel_content(nullptr),
      info_panel(nullptr),
      main_frame(nullptr) {}

  ~UIScreenPrestart() {
    // Supprimer widgets
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

  void update() override {
    // Pas de MAJ periodique necessaire
  }

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

// Initialisation membre statique
UIScreenPrestart* UIScreenPrestart::instance = nullptr;

#endif