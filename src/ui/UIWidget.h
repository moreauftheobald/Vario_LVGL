#ifndef UI_WIDGET_H
#define UI_WIDGET_H

/**
 * @brief Classe de base pour tous les widgets
 * Widget = composant UI reutilisable (vario, altitude, GPS, etc.)
 */
class UIWidget : public UIBase {
protected:
    uint16_t update_interval_ms;  // Intervalle MAJ en ms
    uint32_t last_update_time;    // Timestamp derniere MAJ
    
public:
    /**
     * @brief Constructeur
     * @param interval_ms Intervalle mise a jour (100ms par defaut = 10Hz)
     */
    UIWidget(uint16_t interval_ms = 100) 
        : UIBase(), 
          update_interval_ms(interval_ms), 
          last_update_time(0) {}
    
    virtual ~UIWidget() {}
    
    /**
     * @brief Verifie si widget doit etre mis a jour
     * Utilise throttling base sur intervalle
     * @return true si intervalle depasse
     */
    bool shouldUpdate() {
        uint32_t now = millis();
        if (now - last_update_time >= update_interval_ms) {
            last_update_time = now;
            return true;
        }
        return false;
    }
    
    /**
     * @brief Change l'intervalle de mise a jour
     * @param interval_ms Nouvel intervalle en ms
     */
    void setUpdateInterval(uint16_t interval_ms) {
        update_interval_ms = interval_ms;
    }
    
    /**
     * @brief Obtient l'intervalle actuel
     * @return Intervalle en ms
     */
    uint16_t getUpdateInterval() const {
        return update_interval_ms;
    }
};

#endif