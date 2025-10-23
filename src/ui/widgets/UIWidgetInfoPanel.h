#ifndef UI_WIDGET_INFO_PANEL_H
#define UI_WIDGET_INFO_PANEL_H

/**
 * @brief Widget composite : panel d'informations avec titre et lignes
 * Utilise pour afficher infos systeme, stats, etc.
 */
class UIWidgetInfoPanel : public UIWidget {
private:
    UIWidgetPanel* panel;
    UIWidgetLabel* title_label;
    UIWidgetSeparator* separator;
    std::vector<UIWidgetLabel*> info_labels;
    
    lv_obj_t* content_container;
    
public:
    /**
     * @brief Constructeur
     */
    UIWidgetInfoPanel(uint16_t update_interval_ms = 0)
        : UIWidget(update_interval_ms),
          panel(nullptr),
          title_label(nullptr),
          separator(nullptr),
          content_container(nullptr) {}
    
    ~UIWidgetInfoPanel() {
        // Supprimer widgets
        if (panel) delete panel;
        if (title_label) delete title_label;
        if (separator) delete separator;
        
        for (auto lbl : info_labels) {
            if (lbl) delete lbl;
        }
        info_labels.clear();
    }
    
    /**
     * @brief Cree le panel d'informations
     * @param parent Parent LVGL
     * @param title_text Titre du panel
     * @param width Largeur
     * @param height Hauteur
     */
    void create(lv_obj_t* parent, const char* title_text = "Information",
               int width = 550, int height = 450) {
        
        // Panel principal
        panel = new UIWidgetPanel();
        panel->create(parent, width, height, UIWidgetPanel::STYLE_DEFAULT);
        panel->setPaddingRow(12);
        container = panel->getContainer();
        
        // Conteneur contenu (pour flex column)
        content_container = panel->getPanel();
        lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(content_container, LV_FLEX_ALIGN_START,
                             LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        
        // Titre
        if (title_text) {
            title_label = new UIWidgetLabel();
            title_label->create(content_container, title_text, 
                              UIWidgetLabel::STYLE_SUBTITLE);
            lv_obj_set_width(title_label->getContainer(), LV_PCT(100));
            
            // Separateur sous le titre
            separator = new UIWidgetSeparator();
            separator->create(content_container, UIWidgetSeparator::HORIZONTAL,
                            UIWidgetSeparator::STYLE_MEDIUM);
        }
        
        initialized = true;
    }
    
    /**
     * @brief Methode create() pour compatibilite UIBase
     */
    void create(lv_obj_t* parent) override {
        create(parent, "Information", 550, 450);
    }
    
    /**
     * @brief Ajoute une ligne d'information
     * @param text Texte de la ligne
     * @return Pointeur vers le label cree (pour MAJ ulterieure)
     */
    UIWidgetLabel* addInfoLine(const char* text) {
        if (!content_container) return nullptr;
        
        UIWidgetLabel* lbl = new UIWidgetLabel();
        lbl->create(content_container, text, UIWidgetLabel::STYLE_INFO);
        lv_obj_set_width(lbl->getContainer(), LV_PCT(100));
        
        info_labels.push_back(lbl);
        return lbl;
    }
    
    /**
     * @brief Ajoute une ligne avec icone
     */
    UIWidgetLabel* addInfoLineWithIcon(const char* icon, const char* text) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s %s", icon, text);
        return addInfoLine(buffer);
    }
    
    /**
     * @brief Ajoute un separateur
     */
    void addSeparator() {
        if (!content_container) return;
        
        UIWidgetSeparator* sep = new UIWidgetSeparator();
        sep->create(content_container, UIWidgetSeparator::HORIZONTAL,
                   UIWidgetSeparator::STYLE_DARK);
    }
    
    /**
     * @brief Change le titre
     */
    void setTitle(const char* text) {
        if (title_label) {
            title_label->setText(text);
        }
    }
    
    /**
     * @brief Obtient une ligne d'info par index
     */
    UIWidgetLabel* getInfoLine(size_t index) const {
        if (index < info_labels.size()) {
            return info_labels[index];
        }
        return nullptr;
    }
    
    /**
     * @brief Obtient le nombre de lignes d'info
     */
    size_t getInfoLineCount() const {
        return info_labels.size();
    }
    
    /**
     * @brief Efface toutes les lignes d'info
     */
    void clearInfoLines() {
        for (auto lbl : info_labels) {
            if (lbl) delete lbl;
        }
        info_labels.clear();
    }
    
    /**
     * @brief Met a jour une ligne d'info par index
     */
    void updateInfoLine(size_t index, const char* text) {
        UIWidgetLabel* lbl = getInfoLine(index);
        if (lbl) {
            lbl->setText(text);
        }
    }
    
    /**
     * @brief Obtient le conteneur de contenu (pour ajouts personnalises)
     */
    lv_obj_t* getContentContainer() const {
        return content_container;
    }
    
    /**
     * @brief Obtient le panel sous-jacent
     */
    UIWidgetPanel* getPanel() const {
        return panel;
    }
    
    /**
     * @brief Pas de MAJ periodique par defaut
     */
    void update() override {
        // Widget composite statique
        // Override cette methode si tu veux MAJ auto des infos
    }
};

#endif