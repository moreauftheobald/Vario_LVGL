#ifndef UI_SCREEN_PRESTART_H
#define UI_SCREEN_PRESTART_H

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
    extern UIScreenFileTransfert* file_transfert_screen;
    if (file_transfert_screen) {
      file_transfert_screen->load();
    }
  }

  void onSettingsClicked() {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Settings clicked");
#endif
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
    info_panel = new UIWidgetInfoPanel();
    info_panel->create(panel_content->getPanel(), "Info");
    info_panel->addInfoLine("Vario Ready");

    if (strlen(params.pilot_name) > 0) {
      String name_str = String("Pilot: ") + params.pilot_name;
      info_panel->addInfoLine(name_str.c_str());
    }
    if (params.pilot_phone != "") {
      String phone_str = String("Phone: ") + params.pilot_phone;
      info_panel->addInfoLine(phone_str.c_str());
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

UIScreenPrestart* UIScreenPrestart::instance = nullptr;

#endif