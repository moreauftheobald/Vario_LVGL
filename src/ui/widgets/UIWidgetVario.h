#ifndef UI_WIDGET_VARIO_H
#define UI_WIDGET_VARIO_H

class UIWidgetVario : public UIWidget {
private:
    lv_obj_t* label_value;
    lv_obj_t* chart;
    lv_chart_series_t* series;
    
public:
    UIWidgetVario() : UIWidget(50) {} // Update 20Hz
    
    void create(lv_obj_t* parent) override {
        container = lv_obj_create(parent);
        lv_obj_set_size(container, 200, 150);
        
        // Label valeur
        label_value = lv_label_create(container);
        lv_obj_set_style_text_font(label_value, &lv_font_montserrat_48, 0);
        lv_obj_align(label_value, LV_ALIGN_CENTER, 0, -20);
        
        // Chart historique
        chart = lv_chart_create(container);
        lv_obj_set_size(chart, 180, 60);
        lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, -5);
        series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), 
                                     LV_CHART_AXIS_PRIMARY_Y);
        
        initialized = true;
    }
    
    void update() override {
        if (!shouldUpdate() || !visible) return;
        
        // Lecture vario depuis Kalman filter
        float vario = g_flight_data.vario_filtered;
        
        // Mise a jour label
        char buf[16];
        snprintf(buf, sizeof(buf), "%+.1f", vario);
        lv_label_set_text(label_value, buf);
        
        // Couleur selon signe
        if (vario > 0.5f) {
            lv_obj_set_style_text_color(label_value, 
                lv_color_hex(0x00FF00), 0);
        } else if (vario < -1.0f) {
            lv_obj_set_style_text_color(label_value, 
                lv_color_hex(0xFF0000), 0);
        } else {
            lv_obj_set_style_text_color(label_value, 
                lv_color_hex(0xFFFFFF), 0);
        }
        
        // Mise a jour chart
        lv_chart_set_next_value(chart, series, (int32_t)(vario * 10));
    }
};

#endif