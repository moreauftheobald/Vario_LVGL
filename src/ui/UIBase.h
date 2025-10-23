#ifndef UI_BASE_H
#define UI_BASE_H

/**
 * @brief Classe abstraite de base pour tous les elements UI
 * Fournit interface commune pour ecrans et widgets
 */
class UIBase {
protected:
    lv_obj_t* container;  // Conteneur principal
    bool visible;          // Etat visibilite
    bool initialized;      // Etat initialisation
    
public:
    UIBase() : container(nullptr), visible(false), initialized(false) {}
    
    virtual ~UIBase() {
        if (container) {
            lv_obj_del(container);
            container = nullptr;
        }
    }
    
    /**
     * @brief Cree les objets LVGL (doit etre implementee)
     * @param parent Objet parent LVGL (nullptr = root)
     */
    virtual void create(lv_obj_t* parent) = 0;
    
    /**
     * @brief Met a jour le contenu (doit etre implementee)
     * Appelee regulierement par la task UI
     */
    virtual void update() = 0;
    
    /**
     * @brief Affiche l'element
     */
    virtual void show() {
        if (container && !visible) {
            lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
            visible = true;
        }
    }
    
    /**
     * @brief Cache l'element
     */
    virtual void hide() {
        if (container && visible) {
            lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
            visible = false;
        }
    }
    
    /**
     * @brief Getters
     */
    bool isVisible() const { return visible; }
    bool isInitialized() const { return initialized; }
    lv_obj_t* getContainer() const { return container; }
};

#endif