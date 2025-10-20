#ifndef LANG_H
#define LANG_H

#include <Arduino.h>

typedef enum {
  LANG_FR = 0,
  LANG_EN = 1
} Language;

static Language current_language = LANG_FR;

struct TextStrings {
  const char* file_transfer;
  const char* settings;
  const char* start;
  const char* exit;
  const char* back;
  const char* pilot_settings;
  const char* wifi_settings;
  const char* screen_calibration;
  const char* vario_settings;
  const char* map_settings;
  const char* system_settings;
  const char* save;
  const char* cancel;
  const char* reset;
  const char* pilot_name;
  const char* pilot_firstname;
  const char* pilot_wing;
  const char* pilot_phone;
  const char* wifi_priority;
  const char* wifi_ssid;
  const char* wifi_password;
  const char* vario_integration;
  const char* vario_damping;
  const char* climb_threshold;
  const char* sink_threshold;
  const char* map_zoom;
  const char* map_tile_server;
  const char* map_track_points;
  const char* map_vario_colors;
  const char* map_position_indicator;
  const char* information;
  const char* version;
  const char* sd_card;
  const char* free_space;
  const char* maps;
  const char* flights;
  const char* pilot;
  const char* phone;
};

// Strings FR en PROGMEM
static const char str_fr_file_transfer[] PROGMEM = "Transfert fichiers";
static const char str_fr_settings[] PROGMEM = "Parametres";
static const char str_fr_start[] PROGMEM = "Demarrage";
static const char str_fr_exit[] PROGMEM = "Sortie";
static const char str_fr_back[] PROGMEM = "Retour";
static const char str_fr_pilot_settings[] PROGMEM = "Parametres Pilote";
static const char str_fr_wifi_settings[] PROGMEM = "Parametres WiFi";
static const char str_fr_screen_calibration[] PROGMEM = "Calibration Ecran";
static const char str_fr_vario_settings[] PROGMEM = "Parametres Vario";
static const char str_fr_map_settings[] PROGMEM = "Parametres Cartographie";
static const char str_fr_system_settings[] PROGMEM = "Parametres Systeme";
static const char str_fr_save[] PROGMEM = "Enregistrer";
static const char str_fr_cancel[] PROGMEM = "Annuler";
static const char str_fr_reset[] PROGMEM = "reset";
static const char str_fr_pilot_name[] PROGMEM = "Nom";
static const char str_fr_pilot_firstname[] PROGMEM = "Prenom";
static const char str_fr_pilot_wing[] PROGMEM = "Modele de voile";
static const char str_fr_pilot_phone[] PROGMEM = "Telephone";
static const char str_fr_wifi_priority[] PROGMEM = "Priorite";
static const char str_fr_wifi_ssid[] PROGMEM = "SSID";
static const char str_fr_wifi_password[] PROGMEM = "Mot de passe";
static const char str_fr_vario_integration[] PROGMEM = "Periode d'integration";
static const char str_fr_vario_damping[] PROGMEM = "Amortissement";
static const char str_fr_climb_threshold[] PROGMEM = "Seuil de montee";
static const char str_fr_sink_threshold[] PROGMEM = "Seuil de descente";
static const char str_fr_map_zoom[] PROGMEM = "Niveau de zoom";
static const char str_fr_map_tile_server[] PROGMEM = "Serveur de tuiles";
static const char str_fr_map_track_points[] PROGMEM = "Points de trace";
static const char str_fr_map_vario_colors[] PROGMEM = "Couleurs vario";
static const char str_fr_map_position_indicator[] PROGMEM = "Indicateur de position";
static const char str_fr_information[] PROGMEM = "Informations";
static const char str_fr_version[] PROGMEM = "Version";
static const char str_fr_sd_card[] PROGMEM = "Carte SD";
static const char str_fr_free_space[] PROGMEM = "Espace libre";
static const char str_fr_maps[] PROGMEM = "Cartes";
static const char str_fr_flights[] PROGMEM = "Vols";
static const char str_fr_pilot[] PROGMEM = "Pilote";
static const char str_fr_phone[] PROGMEM = "Telephone";

// Strings EN en PROGMEM
static const char str_en_file_transfer[] PROGMEM = "File Transfer";
static const char str_en_settings[] PROGMEM = "Settings";
static const char str_en_start[] PROGMEM = "Start";
static const char str_en_exit[] PROGMEM = "Exit";
static const char str_en_back[] PROGMEM = "Back";
static const char str_en_pilot_settings[] PROGMEM = "Pilot Settings";
static const char str_en_wifi_settings[] PROGMEM = "WiFi Settings";
static const char str_en_screen_calibration[] PROGMEM = "Screen Calibration";
static const char str_en_vario_settings[] PROGMEM = "Vario Settings";
static const char str_en_map_settings[] PROGMEM = "Map Settings";
static const char str_en_system_settings[] PROGMEM = "System Settings";
static const char str_en_save[] PROGMEM = "Save";
static const char str_en_cancel[] PROGMEM = "Cancel";
static const char str_en_reset[] PROGMEM = "Reset";
static const char str_en_pilot_name[] PROGMEM = "Name";
static const char str_en_pilot_firstname[] PROGMEM = "First Name";
static const char str_en_pilot_wing[] PROGMEM = "Wing Model";
static const char str_en_pilot_phone[] PROGMEM = "Phone";
static const char str_en_wifi_priority[] PROGMEM = "Priority";
static const char str_en_wifi_ssid[] PROGMEM = "SSID";
static const char str_en_wifi_password[] PROGMEM = "Password";
static const char str_en_vario_integration[] PROGMEM = "Integration Period";
static const char str_en_vario_damping[] PROGMEM = "Damping";
static const char str_en_climb_threshold[] PROGMEM = "Climb Threshold";
static const char str_en_sink_threshold[] PROGMEM = "Sink Threshold";
static const char str_en_map_zoom[] PROGMEM = "Zoom Level";
static const char str_en_map_tile_server[] PROGMEM = "Tile Server";
static const char str_en_map_track_points[] PROGMEM = "Track Points";
static const char str_en_map_vario_colors[] PROGMEM = "Vario Colors";
static const char str_en_map_position_indicator[] PROGMEM = "Position Indicator";
static const char str_en_information[] PROGMEM = "Information";
static const char str_en_version[] PROGMEM = "Version";
static const char str_en_sd_card[] PROGMEM = "SD Card";
static const char str_en_free_space[] PROGMEM = "Free Space";
static const char str_en_maps[] PROGMEM = "Maps";
static const char str_en_flights[] PROGMEM = "Flights";
static const char str_en_pilot[] PROGMEM = "Pilot";
static const char str_en_phone[] PROGMEM = "Phone";

// Tables de pointeurs en PROGMEM
static const TextStrings text_fr PROGMEM = {
  str_fr_file_transfer,
  str_fr_settings,
  str_fr_start,
  str_fr_exit,
  str_fr_back,
  str_fr_pilot_settings,
  str_fr_wifi_settings,
  str_fr_screen_calibration,
  str_fr_vario_settings,
  str_fr_map_settings,
  str_fr_system_settings,
  str_fr_save,
  str_fr_cancel,
  str_fr_reset,
  str_fr_pilot_name,
  str_fr_pilot_firstname,
  str_fr_pilot_wing,
  str_fr_pilot_phone,
  str_fr_wifi_priority,
  str_fr_wifi_ssid,
  str_fr_wifi_password,
  str_fr_vario_integration,
  str_fr_vario_damping,
  str_fr_climb_threshold,
  str_fr_sink_threshold,
  str_fr_map_zoom,
  str_fr_map_tile_server,
  str_fr_map_track_points,
  str_fr_map_vario_colors,
  str_fr_map_position_indicator,
  str_fr_information,
  str_fr_version,
  str_fr_sd_card,
  str_fr_free_space,
  str_fr_maps,
  str_fr_flights,
  str_fr_pilot,
  str_fr_phone
};

static const TextStrings text_en PROGMEM = {
  str_en_file_transfer,
  str_en_settings,
  str_en_start,
  str_en_exit,
  str_en_back,
  str_en_pilot_settings,
  str_en_wifi_settings,
  str_en_screen_calibration,
  str_en_vario_settings,
  str_en_map_settings,
  str_en_system_settings,
  str_en_save,
  str_en_cancel,
  str_en_reset,
  str_en_pilot_name,
  str_en_pilot_firstname,
  str_en_pilot_wing,
  str_en_pilot_phone,
  str_en_wifi_priority,
  str_en_wifi_ssid,
  str_en_wifi_password,
  str_en_vario_integration,
  str_en_vario_damping,
  str_en_climb_threshold,
  str_en_sink_threshold,
  str_en_map_zoom,
  str_en_map_tile_server,
  str_en_map_track_points,
  str_en_map_vario_colors,
  str_en_map_position_indicator,
  str_en_information,
  str_en_version,
  str_en_sd_card,
  str_en_free_space,
  str_en_maps,
  str_en_flights,
  str_en_pilot,
  str_en_phone
};

// Fonction helper pour copier depuis PROGMEM vers SRAM
static TextStrings current_text_ram;

static inline const TextStrings* get_text(void) {
  const TextStrings* src = (current_language == LANG_FR) ? &text_fr : &text_en;
  
  // Copier la structure depuis PROGMEM vers SRAM
  memcpy_P(&current_text_ram, src, sizeof(TextStrings));
  
  return &current_text_ram;
}

#endif