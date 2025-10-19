#ifndef LANG_H
#define LANG_H

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

static const TextStrings text_fr = {
  .file_transfer = "Transfert fichiers",
  .settings = "Parametres",
  .start = "Demarrage",
  .exit = "Sortie",
  .back = "Retour",
  .pilot_settings = "Parametres Pilote",
  .wifi_settings = "Parametres WiFi",
  .screen_calibration = "Calibration Ecran",
  .vario_settings = "Parametres Vario",
  .map_settings = "Parametres Cartographie",
  .system_settings = "Parametres Systeme",
  .save = "Enregistrer",
  .cancel = "Annuler",
  .reset = "reset",
  .pilot_name = "Nom",
  .pilot_firstname = "Prenom",
  .pilot_wing = "Modele de voile",
  .pilot_phone = "Telephone",
  .wifi_priority = "Priorite",
  .wifi_ssid = "SSID",
  .wifi_password = "Mot de passe",
  .vario_integration = "Periode d'integration",
  .vario_damping = "Amortissement",
  .climb_threshold = "Seuil de montee",
  .sink_threshold = "Seuil de descente",
  .map_zoom = "Niveau de zoom",
  .map_tile_server = "Serveur de tuiles",
  .map_track_points = "Points de trace",
  .map_vario_colors = "Couleurs vario",
  .map_position_indicator = "Indicateur de position",
  .information = "Informations",
  .version = "Version",
  .sd_card = "Carte SD",
  .free_space = "Espace libre",
  .maps = "Cartes",
  .flights = "Vols",
  .pilot = "Pilote",
  .phone = "Telephone"
};

static const TextStrings text_en = {
  .file_transfer = "File Transfer",
  .settings = "Settings",
  .start = "Start",
  .exit = "Exit",
  .back = "Back",
  .pilot_settings = "Pilot Settings",
  .wifi_settings = "WiFi Settings",
  .screen_calibration = "Screen Calibration",
  .vario_settings = "Vario Settings",
  .map_settings = "Map Settings",
  .system_settings = "System Settings",
  .save = "Save",
  .cancel = "Cancel",
  .reset = "reset",
  .pilot_name = "Last Name",
  .pilot_firstname = "First Name",
  .pilot_wing = "Wing Model",
  .pilot_phone = "Phone",
  .wifi_priority = "Priority",
  .wifi_ssid = "SSID",
  .wifi_password = "Password",
  .vario_integration = "Integration period",
  .vario_damping = "Damping",
  .climb_threshold = "Climb threshold",
  .sink_threshold = "Sink threshold",
  .map_zoom = "Zoom level",
  .map_tile_server = "Tile server",
  .map_track_points = "Track points",
  .map_vario_colors = "Vario colors",
  .map_position_indicator = "Position indicator",
  .information = "Information",
  .version = "Version",
  .sd_card = "SD Card",
  .free_space = "Free space",
  .maps = "Maps",
  .flights = "Flights",
  .pilot = "Pilot",
  .phone = "Phone"
};

static inline const TextStrings* get_text() {
  return (current_language == LANG_FR) ? &text_fr : &text_en;
}

#endif