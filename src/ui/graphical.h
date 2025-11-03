#ifndef GRAPHICAL_H
#define GRAPHICAL_H

/*=============================================================================
 * GRAPHICAL.H - Configuration centralisée de l'interface utilisateur
 * 
 * Ce fichier centralise toutes les valeurs de style (couleurs, dimensions,
 * polices) pour faciliter la personnalisation et la création de thèmes.
 * 
 * Structure:
 * 1. Palette de couleurs
 * 2. Dimensions et espacements
 * 3. Typographie
 * 4. Opacités et effets
 * 5. Configuration par widgets
 *=============================================================================*/


/*=============================================================================
 * 1. PALETTE DE COULEURS
 *=============================================================================*/

/* --- Couleurs principales --- */
#define UI_COLOR_PRIMARY          0x00d4ff    // Bleu cyan (titres, accents)
#define UI_COLOR_BACKGROUND       0x000000    // Noir (fond principal)
#define UI_COLOR_SURFACE          0x1c1c1e    // Gris foncé (surfaces)
#define UI_COLOR_CONTROL_BG       0x0f1520    // Gris bleuté (contrôles)

/* --- Couleurs d'état --- */
#define UI_COLOR_SUCCESS          0x00FF00    // Vert (OK)
#define UI_COLOR_WARNING          0xffaa00    // Orange (Warning)
#define UI_COLOR_ERROR            0xFF0000    // Rouge (Error)
#define UI_COLOR_INFO             0x007aff    // Bleu info

/* --- Couleurs de texte --- */
#define UI_COLOR_TEXT_PRIMARY     0xffffff    // Blanc (texte principal)
#define UI_COLOR_TEXT_SECONDARY   0x8e8e93    // Gris (labels)
#define UI_COLOR_TEXT_DISABLED    0x444444    // Gris sombre (désactivé)

/* --- Couleurs de bordures --- */
#define UI_COLOR_BORDER_PRIMARY   0xffffff    // Blanc
#define UI_COLOR_BORDER_SECONDARY 0x8e8e93    // Gris
#define UI_COLOR_BORDER_INPUT     0x4080a0    // Bleu clair (textarea, dropdown)
#define UI_COLOR_BORDER_PANEL     0x6080a0    // Bleu moyen (colonnes, panels)

/* --- Couleurs des boutons --- */
#define UI_COLOR_BTN_START        0x34c759    // Vert (Start/OK)
#define UI_COLOR_BTN_SETTINGS     0x8e8e93    // Gris (Settings)
#define UI_COLOR_BTN_FILES        0x007aff    // Bleu (Files)
#define UI_COLOR_BTN_CANCEL       0xff3b30    // Rouge (Cancel)
#define UI_COLOR_BTN_RESET        0xff9500    // Orange (Reset)
#define UI_COLOR_BTN_WIFI         0x007aff    // Bleu (WiFi)
#define UI_COLOR_BTN_VARIO        0x4cd964    // Vert clair (Vario)

/* --- Couleurs des switches --- */
#define UI_COLOR_SWITCH_ON        0x34c759    // Vert
#define UI_COLOR_SWITCH_OFF       0x444444    // Gris foncé

/* --- Couleurs spécifiques widgets --- */
#define UI_COLOR_LED_ALERT        0xff3b30    // Rouge LED
#define UI_COLOR_SLIDER_BG        0x2a3f5f    // Fond slider
#define UI_COLOR_AUDIO_CHART_BG   0x1a2035    // Fond graphique audio
#define UI_COLOR_SEPARATOR        0x2a3f5f    // Séparateurs horizontaux
#define UI_COLOR_CHART_BG         0x0f1729    // Fond graphiques/charts
#define UI_COLOR_CHART_LINE       0x404040    // Lignes graphiques
#define UI_COLOR_PANEL_DARK       0x1a2035    // Fond panels sombres
#define UI_COLOR_GPS_MARKER       0x003366    // Marqueur GPS (bleu foncé)
#define UI_COLOR_DISABLED_BTN     0x808080    // Boutons désactivés
#define UI_COLOR_SEPARATOR        0x2a3f5f    // Séparateurs


/*=============================================================================
 * 2. DIMENSIONS ET ESPACEMENTS
 *=============================================================================*/

/* --- Dimensions des conteneurs principaux --- */
#define UI_MAIN_CONTAINER_W       980
#define UI_MAIN_CONTAINER_H       430
#define UI_LEFT_COL_DUAL_W        485

/* --- Dimensions des boutons --- */
#define UI_BTN_PRESTART_W         280         // Boutons écran prestart
#define UI_BTN_PRESTART_H          70
#define UI_BTN_SETTINGS_W         460         // Boutons écrans settings
#define UI_BTN_SETTINGS_H          80

/* --- Dimensions des widgets --- */
#define UI_SWITCH_WIDTH            60
#define UI_SWITCH_HEIGHT           30
#define UI_SWITCH_INDICATOR_PAD     6         // Padding pour l'indicateur switch
#define UI_TEXTAREA_H              50
#define UI_TEXTAREA_W             760
#define UI_SLIDER_VARIO_W         500
#define UI_SLIDER_VARIO_H          20
#define UI_AUDIO_CHART_H          320

/* --- Dimensions spécifiques --- */
#define UI_HEADER_LINE_W          150
#define UI_MAP_PREVIEW_W          450
#define UI_MAP_PREVIEW_H          360
#define UI_MAP_CANVAS_W           440
#define UI_MAP_CANVAS_H           350
#define UI_SEPARATOR_H              1         // Hauteur séparateur horizontal

/* --- Marges et paddings --- */
#define UI_PAD_NONE                 0
#define UI_PAD_SMALL                5
#define UI_PAD_MEDIUM              12
#define UI_PAD_LARGE               20
#define UI_PAD_XLARGE              30

/* --- Paddings des conteneurs principaux --- */
#define UI_MAIN_CONTAINER_PAD_ALL   UI_PAD_NONE
#define UI_MAIN_CONTAINER_PAD_COL  10
#define UI_LEFT_COL_PAD_ALL        UI_PAD_MEDIUM
#define UI_LEFT_COL_PAD_ROW        UI_PAD_MEDIUM
#define UI_BTNS_CONTAINER_PAD_ALL   UI_PAD_NONE
#define UI_BTNS_CONTAINER_PAD_COL   UI_PAD_NONE

/* --- Positions --- */
#define UI_MAIN_CONTAINER_POS_X     0
#define UI_MAIN_CONTAINER_POS_Y    45
#define UI_BTNS_CONTAINER_POS_Y     5
#define UI_TITLE_POS_Y              0
#define UI_TITLE_POS_Y_SPLASH       5

/* --- Bordures --- */
#define UI_BORDER_NONE              0
#define UI_BORDER_THIN              2
#define UI_BORDER_MEDIUM            3

/* --- Coins arrondis --- */
#define UI_RADIUS_NONE              0
#define UI_RADIUS_SMALL             9
#define UI_RADIUS_LARGE            15

/* --- Effets d'ombre --- */
#define UI_SHADOW_WIDTH            30
#define UI_SHADOW_SPREAD            0


/*=============================================================================
 * 3. TYPOGRAPHIE
 *=============================================================================*/

/* --- Polices principales --- */
#define UI_FONT_TITLE             &lv_font_montserrat_48     // Titres écrans
#define UI_FONT_SUBTITLE          &lv_font_montserrat_28     // Sous-titres
#define UI_FONT_LARGE             &lv_font_montserrat_24     // Texte large
#define UI_FONT_NORMAL            &lv_font_montserrat_20     // Texte normal
#define UI_FONT_SMALL             &lv_font_montserrat_18     // Texte petit
#define UI_FONT_DATA              &lv_font_montserrat_32     // Données numériques

/* --- Alias par usage --- */
#define UI_FONT_SCREEN_TITLE      UI_FONT_TITLE
#define UI_FONT_SECTION_TITLE     UI_FONT_SUBTITLE
#define UI_FONT_BUTTON            UI_FONT_NORMAL
#define UI_FONT_LABEL             UI_FONT_NORMAL
#define UI_FONT_VALUE             UI_FONT_NORMAL
#define UI_FONT_INFO              UI_FONT_SMALL
#define UI_FONT_SYMBOL            UI_FONT_LARGE

/*=============================================================================
 * 4. OPACITES ET EFFETS
 *=============================================================================*/

/* --- Niveaux d'opacité --- */
#define UI_OPA_TRANSPARENT        LV_OPA_TRANSP        // 0%
#define UI_OPA_LOW                LV_OPA_20            // 20%
#define UI_OPA_MEDIUM             LV_OPA_50            // 50%
#define UI_OPA_HIGH               LV_OPA_80            // 80%
#define UI_OPA_SOLID              LV_OPA_COVER         // 100%

/* --- Opacités spécifiques --- */
#define UI_OPA_SHADOW             LV_OPA_40            // Ombres
#define UI_OPA_OVERLAY            LV_OPA_60            // Overlays


/*=============================================================================
 * 5. CONFIGURATION PAR WIDGETS
 *=============================================================================*/

/* --- Frames et panneaux --- */
#define UI_FRAME_BG_COLOR         UI_COLOR_BACKGROUND
#define UI_FRAME_BORDER_COLOR     UI_COLOR_BORDER_PRIMARY
#define UI_FRAME_BORDER_WIDTH     UI_BORDER_MEDIUM
#define UI_FRAME_RADIUS           UI_RADIUS_LARGE
#define UI_FRAME_PAD              UI_PAD_LARGE
#define UI_FRAME_SHADOW_WIDTH     UI_SHADOW_WIDTH
#define UI_FRAME_SHADOW_OPA       UI_OPA_SHADOW

#define UI_PANEL_BG_COLOR         UI_COLOR_BACKGROUND
#define UI_PANEL_BORDER_COLOR     UI_COLOR_BORDER_PRIMARY
#define UI_PANEL_BORDER_WIDTH     UI_BORDER_MEDIUM
#define UI_PANEL_RADIUS           UI_RADIUS_SMALL
#define UI_PANEL_PAD              UI_PAD_LARGE

/* --- Dropdown (liste déroulante) --- */
#define UI_DROPDOWN_BG_COLOR      UI_COLOR_CONTROL_BG
#define UI_DROPDOWN_TEXT_COLOR    UI_COLOR_TEXT_PRIMARY
#define UI_DROPDOWN_SELECTED_BG   UI_COLOR_CONTROL_BG
#define UI_DROPDOWN_CHECKED_BG    UI_COLOR_PRIMARY
#define UI_DROPDOWN_FONT          UI_FONT_NORMAL

/* --- Slider --- */
#define UI_SLIDER_MAIN_BG         UI_COLOR_SLIDER_BG
#define UI_SLIDER_KNOB_COLOR      UI_COLOR_PRIMARY
#define UI_SLIDER_INDICATOR_COLOR UI_COLOR_PRIMARY

/* --- Switch --- */
#define UI_SWITCH_BG_OFF          UI_COLOR_SWITCH_OFF
#define UI_SWITCH_BG_ON           UI_COLOR_SWITCH_ON
#define UI_SWITCH_KNOB_COLOR      UI_COLOR_TEXT_PRIMARY
#define UI_SWITCH_RADIUS          UI_RADIUS_LARGE
#define UI_SWITCH_KNOB_PAD        2

/* --- Map (carte) --- */
#define UI_MAP_CONTAINER_BG       UI_COLOR_SURFACE
#define UI_MAP_BORDER_COLOR       UI_COLOR_PRIMARY
#define UI_MAP_BORDER_WIDTH       UI_BORDER_THIN
#define UI_MAP_PAD                UI_PAD_SMALL

/* --- Separator (séparateur) --- */
#define UI_SEPARATOR_COLOR        UI_COLOR_TEXT_SECONDARY
#define UI_SEPARATOR_WIDTH        UI_SEPARATOR_H

/* --- Status indicators (LED, icônes) --- */
#define UI_STATUS_OK_COLOR        UI_COLOR_SUCCESS
#define UI_STATUS_ERROR_COLOR     UI_COLOR_ERROR
#define UI_STATUS_WARNING_COLOR   UI_COLOR_WARNING
#define UI_STATUS_INFO_COLOR      UI_COLOR_INFO


/*=============================================================================
 * NOTES D'UTILISATION
 *=============================================================================
 * 
 * Pour créer un nouveau thème:
 * 1. Dupliquer ce fichier en graphical_theme_xxx.h
 * 2. Modifier uniquement les valeurs des couleurs et polices
 * 3. Garder les noms de defines identiques
 * 4. Inclure le nouveau fichier au lieu de graphical.h
 * 
 * Exemple de modification pour un thème sombre/bleu:
 * #define UI_COLOR_PRIMARY    0x0080ff  // Bleu plus foncé
 * #define UI_COLOR_SURFACE    0x0a0a0a  // Noir plus profond
 * 
 *=============================================================================*/

#endif // GRAPHICAL_H