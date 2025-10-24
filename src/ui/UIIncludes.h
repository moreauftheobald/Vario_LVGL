#ifndef UI_INCLUDES_H
#define UI_INCLUDES_H

#include <vector>

// ============================================================================
// CLASSES DE BASE
// ============================================================================
#include "UIBase.h"
#include "UIWidget.h"
#include "UIScreen.h"
//#include "UIManager.h"

// ============================================================================
// WIDGETS DE BASE (dans l'ordre alphabetique)
// ============================================================================
#include "widgets/UIWidgetButton.h"
#include "widgets/UIWidgetCheckbox.h"
#include "widgets/UIWidgetDropdown.h"
#include "widgets/UIWidgetLabel.h"
#include "widgets/UIWidgetPanel.h"
#include "widgets/UIWidgetKeyboard.h"
#include "widgets/UIWidgetSeparator.h"
#include "widgets/UIWidgetSlider.h"
#include "widgets/UIWidgetSwitch.h"
#include "widgets/UIWidgetTextArea.h"

// ============================================================================
// WIDGETS COMPOSITES
// ============================================================================
#include "widgets/UIWidgetButtonPair.h"
#include "widgets/UIWidgetFormRow.h"
#include "widgets/UIWidgetInfoPanel.h"
#include "widgets/UIWidgetKeyboard.h"
#include "widgets/UIWidgetScreenFrame.h"

// ============================================================================
// ECRANS (dans l'ordre d'utilisation)
// ============================================================================
#include "screens/UIScreenSplash.h"
#include "screens/UIScreenPrestart.h"

// TODO: Ajouter les futurs ecrans ici au fur et a mesure
// #include "screens/UIScreenSettings.h"
// #include "screens/UIScreenFileTransfer.h"
// #include "screens/UIScreenMain.h"
// #include "screens/UIScreenMap.h"

// ============================================================================
// WIDGETS METIER VARIO (a creer progressivement)
// ============================================================================
// TODO: Decommenter au fur et a mesure de la creation
// #include "widgets/UIWidgetVario.h"
// #include "widgets/UIWidgetAltitude.h"
// #include "widgets/UIWidgetSpeed.h"
// #include "widgets/UIWidgetGPS.h"
// #include "widgets/UIWidgetBattery.h"
// #include "widgets/UIWidgetStatusBar.h"
// #include "widgets/UIWidgetCompass.h"
// #include "widgets/UIWidgetGMeter.h"
// #include "widgets/UIWidgetWind.h"

#endif