#include "pluginlouvre.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

#include <ofnx/files/arnvit.h>
#include <ofnx/tools/log.h>

constexpr int INVENTORY_SIZE = 8;
constexpr int CHEST_SIZE = 128;

// Inventory action button images
const std::string INV_ACTION_BUTTON_USE_DISABLED = "Bout0001.bmp";
const std::string INV_ACTION_BUTTON_USE_ENABLED = "Bout0011.bmp";
const std::string INV_ACTION_BUTTON_USE_SELECTED = "Bout0021.bmp";

const std::string INV_ACTION_BUTTON_SEE_DISABLED = "Bout0002.bmp";
const std::string INV_ACTION_BUTTON_SEE_ENABLED = "Bout0012.bmp";
const std::string INV_ACTION_BUTTON_SEE_SELECTED = "Bout0022.bmp";

const std::string INV_ACTION_BUTTON_COMBINE_DISABLED = "Bout0003.bmp";
const std::string INV_ACTION_BUTTON_COMBINE_ENABLED = "Bout0013.bmp";
const std::string INV_ACTION_BUTTON_COMBINE_SELECTED = "Bout0023.bmp";

const std::string INV_ACTION_BUTTON_SEPARATE_DISABLED = "Bout0004.bmp";
const std::string INV_ACTION_BUTTON_SEPARATE_ENABLED = "Bout0014.bmp";
const std::string INV_ACTION_BUTTON_SEPARATE_SELECTED = "Bout0024.bmp";

constexpr int INV_ACTION_BUTTON_OFFSET_X_USE = 21;
constexpr int INV_ACTION_BUTTON_OFFSET_Y_USE = 145;

constexpr int INV_ACTION_BUTTON_OFFSET_X_SEE = 22;
constexpr int INV_ACTION_BUTTON_OFFSET_Y_SEE = 191;

constexpr int INV_ACTION_BUTTON_OFFSET_X_COMBINE = 25;
constexpr int INV_ACTION_BUTTON_OFFSET_Y_COMBINE = 270;

constexpr int INV_ACTION_BUTTON_OFFSET_X_SEPARATE = 24;
constexpr int INV_ACTION_BUTTON_OFFSET_Y_SEPARATE = 318;

// Offsets for each inventory slot on screen
constexpr int PORTEF_OFFSETS_X[INVENTORY_SIZE] = { 380, 476, 564, 452, 548, 468, 557, 548 };
constexpr int PORTEF_OFFSETS_Y[INVENTORY_SIZE] = { 27, 27, 27, 114, 122, 203, 212, 298 };

// Offsets for selected inventory slot on screen
constexpr int PORTEF_OFFSET_X_SELEC = 103;
constexpr int PORTEF_OFFSET_Y_SELEC = 122;

// Inventory slot selection images
const std::string INV_SLOT_SELECTED_GREY[INVENTORY_SIZE] = {
    "DSel0001.bmp",
    "DSel0002.bmp",
    "DSel0003.bmp",
    "DSel0004.bmp",
    "DSel0005.bmp",
    "DSel0006.bmp",
    "DSel0007.bmp",
    "DSel0008.bmp",
};
const std::string INV_SLOT_SELECTED_GREEN[INVENTORY_SIZE] = {
    "Sel0001.bmp",
    "Sel0002.bmp",
    "Sel0003.bmp",
    "Sel0004.bmp",
    "Sel0005.bmp",
    "Sel0006.bmp",
    "Sel0007.bmp",
    "Sel0008.bmp"
};
const std::string INV_SLOT_SELECTED_RED[INVENTORY_SIZE] = {
    "Sel0101.bmp",
    "Sel0102.bmp",
    "Sel0103.bmp",
    "Sel0104.bmp",
    "Sel0105.bmp",
    "Sel0106.bmp",
    "Sel0107.bmp",
    "Sel0108.bmp"
};
constexpr int INV_SLOT_SELECTED_OFFSET_X[INVENTORY_SIZE] = { 389, 482, 574, 456, 556, 476, 566, 552 };
constexpr int INV_SLOT_SELECTED_OFFSET_Y[INVENTORY_SIZE] = { 82, 83, 84, 170, 178, 258, 268, 352 };

// Inventory statues
const std::string INV_STATUES_SELECTED_GREY[4] = {
    "DSel0009.bmp",
    "DSel0010.bmp",
    "DSel0011.bmp",
    "DSel0012.bmp",
};
const std::string INV_STATUES_SELECTED_GREEN[4] = {
    "Sel0009.bmp",
    "Sel0010.bmp",
    "Sel0011.bmp",
    "Sel0012.bmp",
};
const std::string INV_STATUES_SELECTED_RED[4] = {
    "Sel0109.bmp",
    "Sel0110.bmp",
    "Sel0111.bmp",
    "Sel0112.bmp",
};
constexpr int INV_STATUES_SELECTED_OFFSET_X[4] = { 8, 93, 182, 264 };
constexpr int INV_STATUES_SELECTED_OFFSET_Y[4] = { 32, 32, 27, 28 };
constexpr int INV_STATUES_IMG_OFFSET_X_LEO = 104;
constexpr int INV_STATUES_IMG_OFFSET_Y_LEO = 20;
constexpr int INV_STATUES_IMG_OFFSET_X_DEMON = 193;
constexpr int INV_STATUES_IMG_OFFSET_Y_DEMON = 12;
constexpr int INV_STATUES_IMG_OFFSET_X_VASE = 18;
constexpr int INV_STATUES_IMG_OFFSET_Y_VASE = 13;
constexpr int INV_STATUES_IMG_OFFSET_X_TAURUS = 273;
constexpr int INV_STATUES_IMG_OFFSET_Y_TAURUS = 15;

// Inventory text description
const std::string INV_TEXT_BACKGROUND = "EffaceText.bmp";
constexpr int INV_TEXT_OFFSET_X = 62;
constexpr int INV_TEXT_OFFSET_Y = 402;

// Objects
struct ObjectData {
    std::string imgInventory; // ObjXXX1.bmp
    std::string imgInventorySelected; // ObjXXX0.bmp
    std::string imgChest;

    bool canUse;
    bool canSee;

    std::vector<int> separeTo; // Combined objects this can separe to (none if empty)
    // TODO: manage combinations
};
// TODO: set correct values
std::map<int, ObjectData> g_objectMap = {
    { 100, { "Obj0101.bmp", "Obj0100.bmp", "Cof0101.bmp", false, false, {} } }, // Lion brule-parfum
    { 200, { "Obj0201.bmp", "Obj0200.bmp", "Cof0201.bmp", false, false, {} } }, // Demon assyrien Pazuzu
    { 300, { "Obj0301.bmp", "Obj0300.bmp", "Cof0301.bmp", false, false, {} } }, // Vase surmonte d'un aigle romain
    { 400, { "Obj0401.bmp", "Obj0400.bmp", "Cof0401.bmp", false, false, {} } }, // Figurine assyrienne de fondation en forme de Taureau
    { 500, { "Obj0501.bmp", "Obj0500.bmp", "Cof0501.bmp", false, false, {} } }, // Arbalete legere en kevlar
    { 600, { "Obj0601.bmp", "Obj0600.bmp", "Cof0601.bmp", false, false, {} } }, // Corde en nylon de 9 mm de diametre
    { 700, { "Obj0701.bmp", "Obj0700.bmp", "Cof0701.bmp", false, false, {} } }, // Grappin
    { 800, { "Obj0801.bmp", "Obj0800.bmp", "Cof0801.bmp", false, false, {} } }, // Masque a gaz
    { 900, { "Obj0901.bmp", "Obj0900.bmp", "Cof0901.bmp", false, false, {} } }, // Lampe torche
    { 1000, { "Obj1001.bmp", "Obj1000.bmp", "Cof1001.bmp", false, false, {} } }, // Couteau de diamant
    { 1100, { "Obj1101.bmp", "Obj1100.bmp", "Cof1101.bmp", false, false, {} } }, // Crampon
    { 1200, { "Obj1201.bmp", "Obj1200.bmp", "Cof1201.bmp", false, false, {} } }, // Carreaux d'arbalete
    { 1300, { "Obj1301.bmp", "Obj1300.bmp", "Cof1301.bmp", false, false, { 500, 1200 } } }, // Arbalete en kevlar munie d'un carreau
    { 1400, { "Obj1401.bmp", "Obj1400.bmp", "Cof1401.bmp", false, false, { 500, 600 } } }, // Arbalete en kevlar equipee d'une corde
    { 1500, { "Obj1501.bmp", "Obj1500.bmp", "Cof1501.bmp", false, false, { 500, 600, 700 } } }, // Arbalete lance-grappin
    { 1600, { "Obj1601.bmp", "Obj1600.bmp", "Cof1601.bmp", false, false, { 500, 700 } } }, // Arbalete en kevlar equipee d'un grappin
    { 1700, { "Obj1701.bmp", "Obj1700.bmp", "Cof1701.bmp", false, false, {} } }, // Salpetre
    { 1800, { "Obj1801.bmp", "Obj1800.bmp", "Cof1801.bmp", false, false, {} } }, // Bouteille de Fleur d'Aconit
    { 1900, { "Obj1901.bmp", "Obj1900.bmp", "Cof1901.bmp", false, false, {} } }, // Cle en bronze
    { 2000, { "Obj2001.bmp", "Obj2000.bmp", "Cof2001.bmp", false, false, {} } }, // Cruche remplie d'eau
    { 2100, { "Obj2101.bmp", "Obj2100.bmp", "Cof2101.bmp", false, false, {} } }, // Bouteille d'eau-de-vie
    { 2200, { "Obj2201.bmp", "Obj2200.bmp", "Cof2201.bmp", false, false, {} } }, // Safran
    { 2300, { "Obj2301.bmp", "Obj2300.bmp", "Cof2301.bmp", false, false, {} } }, // Rose
    { 2400, { "Obj2401.bmp", "Obj2400.bmp", "Cof2401.bmp", false, false, {} } }, // Cle en or
    { 2500, { "Obj2501.bmp", "Obj2500.bmp", "Cof2501.bmp", false, false, {} } }, // Rubis du sceptre de Charles V
    { 2600, { "Obj2601.bmp", "Obj2600.bmp", "Cof2601.bmp", false, false, {} } }, // Encens
    { 2700, { "Obj2701.bmp", "Obj2700.bmp", "Cof2701.bmp", false, false, {} } }, // Cle en argent
    { 2800, { "Obj2801.bmp", "Obj2800.bmp", "Cof2801.bmp", false, false, {} } }, // Tabouret
    { 2900, { "Obj2901.bmp", "Obj2900.bmp", "Cof2901.bmp", false, false, {} } }, // Plaque d'ivoire utile dans le cadre des missives secretes royales
    { 3000, { "Obj3001.bmp", "Obj3000.bmp", "Cof3001.bmp", false, false, {} } }, // Parchemin royal code
    { 3100, { "Obj3101.bmp", "Obj3100.bmp", "Cof3101.bmp", false, false, {} } }, // Parchemin royal decode
    { 3200, { "Obj3201.bmp", "Obj3200.bmp", "Cof3201.bmp", false, false, {} } }, // Petite cle de meuble sertie de diamants
    { 3300, { "Obj3301.bmp", "Obj3300.bmp", "Cof3301.bmp", false, false, {} } }, // Petite cle pour fermoir de manuscrit
    { 3400, { "Obj3401.bmp", "Obj3400.bmp", "Cof3401.bmp", false, false, {} } }, // Formule magique de revelation
    { 3500, { "Obj3501.bmp", "Obj3500.bmp", "Cof3501.bmp", false, false, {} } }, // Pate rouge
    { 3600, { "Obj3601.bmp", "Obj3600.bmp", "Cof3601.bmp", false, false, {} } }, // Pate bleue
    { 3700, { "Obj3701.bmp", "Obj3700.bmp", "Cof3701.bmp", false, false, {} } }, // Graines jaunes
    { 3800, { "Obj3801.bmp", "Obj3800.bmp", "Cof3801.bmp", false, false, {} } }, // Graines rouges
    { 3900, { "Obj3901.bmp", "Obj3900.bmp", "Cof3901.bmp", false, false, {} } }, // Pastille d'antimoine
    { 4000, { "Obj4001.bmp", "Obj4000.bmp", "Cof4001.bmp", false, false, {} } }, // Pierre
    { 4100, { "Obj4101.bmp", "Obj4100.bmp", "Cof4101.bmp", false, false, {} } }, // Cigue
    { 4200, { "Obj4201.bmp", "Obj4200.bmp", "Cof4201.bmp", false, false, {} } }, // Missive redigee a l'encre sympathique
    { 4300, { "Obj4301.bmp", "Obj4300.bmp", "Cof4301.bmp", false, false, {} } }, // Missive fort compromettante !!
    { 4400, { "Obj4401.bmp", "Obj4400.bmp", "Cof4401.bmp", false, false, {} } }, // Pistolet des gens de Monsieur de La Force
    { 4500, { "Obj4501.bmp", "Obj4500.bmp", "Cof4501.bmp", false, false, {} } }, // Livres royaux
    { 4600, { "Obj4601.bmp", "Obj4600.bmp", "Cof4601.bmp", false, false, {} } }, // Cle de desserte
    { 4700, { "Obj4701.bmp", "Obj4700.bmp", "Cof4701.bmp", false, false, {} } }, // Cle de monte-charge
    { 4800, { "Obj4801.bmp", "Obj4800.bmp", "Cof4801.bmp", false, false, {} } }, // Sceptre royal
    { 4900, { "Obj4901.bmp", "Obj4900.bmp", "Cof4901.bmp", false, false, {} } }, // Marteau et burin
    { 5000, { "Obj5001.bmp", "Obj5000.bmp", "Cof5001.bmp", false, false, {} } }, // Flasque offerte par l'alchimiste remplie d'une solution neutralisante... On ne sait jamais
    { 5100, { "Obj5101.bmp", "Obj5100.bmp", "Cof5101.bmp", false, false, {} } }, // Formule d'ouverture
    { 5200, { "Obj5201.bmp", "Obj5200.bmp", "Cof5201.bmp", false, false, {} } }, // Panier de linge
    { 5300, { "Obj5301.bmp", "Obj5300.bmp", "Cof5301.bmp", false, false, {} } }, // Chevaliere royale
    { 5400, { "Obj5401.bmp", "Obj5400.bmp", "Cof5401.bmp", false, false, {} } }, // Flasque pleine d'urine
    { 5500, { "Obj5501.bmp", "Obj5500.bmp", "Cof5501.bmp", false, false, {} } }, // Note royale devoilant l'entree d'un passage secret
    { 5600, { "Obj5601.bmp", "Obj5600.bmp", "Cof5601.bmp", false, false, {} } }, // Pince a feu
    { 5700, { "Obj5701.bmp", "Obj5700.bmp", "Cof5701.bmp", false, false, {} } }, // Buche
    { 5800, { "Obj5801.bmp", "Obj5800.bmp", "Cof5801.bmp", false, false, {} } }, // Cle zodiacale en forme de Taureau
    { 5900, { "Obj5901.bmp", "Obj5900.bmp", "Cof5901.bmp", false, false, {} } }, // Plans de l'horloge allemande realises par le Sieur Gautier
    { 6000, { "Obj6001.bmp", "Obj6000.bmp", "Cof6001.bmp", false, false, {} } }, // Poire a poudre
    { 6100, { "Obj6101.bmp", "Obj6100.bmp", "Cof6101.bmp", false, false, {} } }, // Balles de pistolet en plomb
    { 6200, { "Obj6201.bmp", "Obj6200.bmp", "Cof6201.bmp", false, false, {} } }, // Pistolet et balles
    { 6300, { "Obj6301.bmp", "Obj6300.bmp", "Cof6301.bmp", false, false, {} } }, // Pistolet rempli de poudre
    { 6400, { "Obj6401.bmp", "Obj6400.bmp", "Cof6401.bmp", false, false, {} } }, // Pistolet rempli de poudre et charge, pret a fonctionner
    { 6500, { "Obj6501.bmp", "Obj6500.bmp", "Cof6501.bmp", false, false, {} } }, // Fil de cuivre
    { 6600, { "Obj6601.bmp", "Obj6600.bmp", "Cof6601.bmp", false, false, {} } }, // Bourse bien garnie
    { 6700, { "Obj6701.bmp", "Obj6700.bmp", "Cof6701.bmp", false, false, {} } }, // Faitage en cuivre
    { 6800, { "Obj6801.bmp", "Obj6800.bmp", "Cof6801.bmp", false, false, {} } }, // Tuile de zinc
    { 6900, { "Obj6901.bmp", "Obj6900.bmp", "Cof6901.bmp", false, false, {} } }, // Collier de perles
    { 7000, { "Obj7001.bmp", "Obj7000.bmp", "Cof7001.bmp", false, false, {} } }, // Formule magique devoilant certains secrets fondamentaux d'Anselme
    { 7100, { "Obj7101.bmp", "Obj7100.bmp", "Cof7101.bmp", false, false, {} } }, // Partition de Bastien et Bastienne, premier opera de Wolfgang Amadeus Mozart
    { 7200, { "Obj7201.bmp", "Obj7200.bmp", "Cof7201.bmp", false, false, {} } }, // Cle de coeur
    { 7300, { "Obj7301.bmp", "Obj7300.bmp", "Cof7301.bmp", false, false, {} } }, // Flasque de sulfate
    { 7400, { "Obj7401.bmp", "Obj7400.bmp", "Cof7401.bmp", false, false, {} } }, // Lamelle de zinc
    { 7500, { "Obj7501.bmp", "Obj7500.bmp", "Cof7501.bmp", false, false, {} } }, // Lamelle de cuivre
    { 7600, { "Obj7601.bmp", "Obj7600.bmp", "Cof7601.bmp", false, false, {} } }, // Memoires sur l'electricite
    { 7700, { "Obj7701.bmp", "Obj7700.bmp", "Cof7701.bmp", false, false, {} } }, // Notes d'une Expedition scientifique en Perse
    { 7800, { "Obj7801.bmp", "Obj7800.bmp", "Cof7801.bmp", false, false, {} } }, // Notes d'une Expedition scientifique en Perse devoilant un secret
    { 7900, { "Obj7901.bmp", "Obj7900.bmp", "Cof7901.bmp", false, false, {} } }, // Bouteille de Leyde
    { 8000, { "Obj8001.bmp", "Obj8000.bmp", "Cof8001.bmp", false, false, {} } }, // Manivelle de generateur
    { 8100, { "Obj8101.bmp", "Obj8100.bmp", "Cof8101.bmp", false, false, {} } }, // Bouteille de Leyde chargee en electricite
    { 8200, { "Obj8201.bmp", "Obj8200.bmp", "Cof8201.bmp", false, false, {} } }, // Flute de champagne
    { 8300, { "Obj8301.bmp", "Obj8300.bmp", "Cof8301.bmp", false, false, {} } }, // Cle de bibliotheque
    { 8400, { "Obj8401.bmp", "Obj8400.bmp", "Cof8401.bmp", false, false, {} } }, // Flasque pleine d'acide
    { 8500, { "Obj8501.bmp", "Obj8500.bmp", "Cof8501.bmp", false, false, {} } }, // Portrait de Christophe Gabriel Allegrain
    { 8600, { "Obj8601.bmp", "Obj8600.bmp", "Cof8601.bmp", false, false, {} } }, // Portrait de Christophe Allegrain par Duplessis enrichi d'un maillet et d'un ciseau
    { 8700, { "Obj8701.bmp", "Obj8700.bmp", "Cof8701.bmp", false, false, {} } }, // Escabeau
    { 8800, { "Obj8801.bmp", "Obj8800.bmp", "Cof8801.bmp", false, false, {} } }, // Catalogue de l'exposition du Salon Carre
    { 8900, { "Obj8901.bmp", "Obj8900.bmp", "Cof8901.bmp", false, false, {} } }, // Sac de charbon
    { 9000, { "Obj9001.bmp", "Obj9000.bmp", "Cof9001.bmp", false, false, {} } }, // Reconstruction de la statuette du demon assyrien Pazuzu.
    { 9100, { "Obj9101.bmp", "Obj9100.bmp", "Cof9101.bmp", false, false, {} } }, // Vaisselle en etain
    { 9200, { "Obj9201.bmp", "Obj9200.bmp", "Cof9201.bmp", false, false, {} } }, // Maillet de sculpteur
    { 9300, { "Obj9301.bmp", "Obj9300.bmp", "Cof9301.bmp", false, false, {} } }, // Fibule magique realisee par Anselme
    { 9400, { "Obj9401.bmp", "Obj9400.bmp", "Cof9401.bmp", false, false, {} } }, //
    { 9500, { "Obj9501.bmp", "Obj9500.bmp", "Cof9501.bmp", false, false, {} } }, // Flute de champagne assortie d'un precieux collier de perles
    { 9600, { "Obj9601.bmp", "Obj9600.bmp", "Cof9601.bmp", false, false, {} } }, // Arbalete en kevlar munie d'un grappin
    { 9700, { "Obj9701.bmp", "Obj9700.bmp", "Cof9701.bmp", true, false, {} } }, // Carte magnetique bleue
    { 9800, { "Obj9801.bmp", "Obj9800.bmp", "Cof9801.bmp", false, false, {} } }, // Champagne et collier de perles
};

struct LouvreData {
    int objectInventory[INVENTORY_SIZE] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int selectedObjectSlot = -1;

    int objectChest[CHEST_SIZE] = { -1 };

    std::string isMonde4 = "0.0";

    ofnx::files::ArnVit arnVit;
};

LouvreData g_louvreData;

/* Helper functions */
void drawImageToScreen(Engine& engine, const std::string& img, int x, int y)
{
    ofnx::files::ArnVit::ArnVitFile file = g_louvreData.arnVit.getFile(img);
    if (file.data.empty()) {
        LOG_ERROR("Unable to read image file: {}", img);
        return;
    }

    std::vector<uint16_t>& fb = engine.getFrameBuffer();
    uint16_t* imgData = (uint16_t*)file.data.data();
    for (int i = 0; i < file.height; ++i) {
        for (int j = 0; j < file.width; ++j) {
            fb[(y + i) * 640 + x + j] = imgData[i * file.width + j];
        }
    }
}

void printPortefSelectedObject(Engine& engine, int objectSlot)
{
    int objectId = g_louvreData.objectInventory[objectSlot];

    // Print object selection
    drawImageToScreen(engine, g_objectMap[objectId].imgInventorySelected, PORTEF_OFFSET_X_SELEC, PORTEF_OFFSET_Y_SELEC);

    // Print object description
    drawImageToScreen(engine, INV_TEXT_BACKGROUND, INV_TEXT_OFFSET_X, INV_TEXT_OFFSET_Y);

    // Print action buttons
    // Can use
    if (g_objectMap[objectId].canUse) {
        drawImageToScreen(engine, INV_ACTION_BUTTON_USE_ENABLED, INV_ACTION_BUTTON_OFFSET_X_USE, INV_ACTION_BUTTON_OFFSET_Y_USE);
    } else {
        drawImageToScreen(engine, INV_ACTION_BUTTON_USE_DISABLED, INV_ACTION_BUTTON_OFFSET_X_USE, INV_ACTION_BUTTON_OFFSET_Y_USE);
    }

    // Can see
    if (g_objectMap[objectId].canSee) {
        drawImageToScreen(engine, INV_ACTION_BUTTON_SEE_ENABLED, INV_ACTION_BUTTON_OFFSET_X_SEE, INV_ACTION_BUTTON_OFFSET_Y_SEE);
    } else {
        drawImageToScreen(engine, INV_ACTION_BUTTON_SEE_DISABLED, INV_ACTION_BUTTON_OFFSET_X_SEE, INV_ACTION_BUTTON_OFFSET_Y_SEE);
    }

    // TODO: implement combine action

    // Separate
    if (!g_objectMap[objectId].separeTo.empty()) {
        drawImageToScreen(engine, INV_ACTION_BUTTON_SEPARATE_ENABLED, INV_ACTION_BUTTON_OFFSET_X_SEPARATE, INV_ACTION_BUTTON_OFFSET_Y_SEPARATE);
    } else {
        drawImageToScreen(engine, INV_ACTION_BUTTON_SEPARATE_DISABLED, INV_ACTION_BUTTON_OFFSET_X_SEPARATE, INV_ACTION_BUTTON_OFFSET_Y_SEPARATE);
    }

    // Print slot selection
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (i == objectSlot) {
            drawImageToScreen(engine, INV_SLOT_SELECTED_GREEN[i], INV_SLOT_SELECTED_OFFSET_X[i], INV_SLOT_SELECTED_OFFSET_Y[i]);
        } else {
            drawImageToScreen(engine, INV_SLOT_SELECTED_GREY[i], INV_SLOT_SELECTED_OFFSET_X[i], INV_SLOT_SELECTED_OFFSET_Y[i]);
        }
    }
}

/* Plugin */
void plgPlayMovie(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string movie = args[0];

    engine.playMovie(movie);
}

void plgCmp(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 5) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string cond = args[0];
    std::string notCond = args[1];
    std::string var = args[2];
    std::string op = args[3];
    double value = std::stod(args[4]);
    bool result = false;
    if (op == "==") {
        result = std::stod(engine.getStateValue(var)) == value;
    } else if (op == "!=") {
        result = std::stod(engine.getStateValue(var)) != value;
    } else if (op == "<") {
        result = std::stod(engine.getStateValue(var)) < value;
    } else if (op == "<=") {
        result = std::stod(engine.getStateValue(var)) <= value;
    } else if (op == ">") {
        result = std::stod(engine.getStateValue(var)) > value;
    } else if (op == ">=") {
        result = std::stod(engine.getStateValue(var)) >= value;
    } else {
        LOG_ERROR("Invalid operator: {}", op);
    }

    engine.setStateValue(cond, result ? "1.0" : "0.0");
    engine.setStateValue(notCond, result ? "0.0" : "1.0");
}

void plgKillTimer(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgCarteDestination(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string x = args[0];
    std::string y = args[1];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgPauseTimer(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double timer = std::stod(args[0]);
    double value = std::stod(args[1]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgAdd(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string object = args[0];
    std::string cond = args[1];
    std::string notCond = args[2];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgInitCoffre(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgAffichePortef(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (g_louvreData.objectInventory[i] == -1) {
            // Skip empty slots
            continue;
        }

        int objectId = g_louvreData.objectInventory[i];

        drawImageToScreen(engine, g_objectMap[objectId].imgInventory, PORTEF_OFFSETS_X[i], PORTEF_OFFSETS_Y[i]);
    }
}

void plgAfficheSelection(Engine& engine, std::vector<std::string> args)
{
    if (g_louvreData.selectedObjectSlot != -1) {
        printPortefSelectedObject(engine, g_louvreData.selectedObjectSlot);
    }
}

void plgAfficheCoffre(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgScroll(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSelectPorteF(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string z = args[1];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgPlayAnimBloc(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 4) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string name = args[0];
    std::string var = args[1];
    double frameCount = std::stod(args[2]);
    double speed = std::stod(args[3]);

    // TODO: implement correctly
    engine.playAnim(name);
}

void plgUntilLoop(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string variable = args[0];
    double value = std::stod(args[1]);

    engine.untilLoop(variable, value);
}

void plgMemoryRelease(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSelectCoffre(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string z = args[1];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgPlayAnimBlocNumber(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 5) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string value = args[0];
    std::string y = args[1];
    std::string z = args[2];
    double value1 = std::stod(args[3]);
    double value2 = std::stod(args[4]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSub(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string value = args[0];
    std::string y = args[1];
    double value2 = std::stod(args[2]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgWhileLoop(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double timer = std::stod(args[0]);

    engine.whileLoop(timer);
}

void plgIsHere(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgDrawTextSelection(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement

    // Draws text in box: x1=62, y1=405, x2=354, y2464

    LOG_ERROR("Not implemented");
}

void plgPorteFRollover(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSetMonde4(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    g_louvreData.isMonde4 = value;
}

void plgGetMonde4(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string cond = args[0];
    std::string notCond = args[1];

    engine.setStateValue(cond, g_louvreData.isMonde4);
    engine.setStateValue(notCond, g_louvreData.isMonde4);
}

void plgChangeCurseur(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveContextRestored(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string reloading = args[0];
    std::string reloadDone = args[1];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveCaptureContext(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveInitSlots(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveSave(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgMultiCdSetNextScript(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string value = args[0];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgReset(Engine& engine, std::vector<std::string> args)
{
    for (int i = 0; i < CHEST_SIZE; ++i) {
        g_louvreData.objectChest[i] = 0;
    }
    for (int i = 0; i < 13; ++i) {
        engine.setStateValue("pos" + std::to_string(i), "0");
    }
}

void plgAddObject(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string cond = args[1];
    std::string notCond = args[2];

    bool inserted = false;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (g_louvreData.objectInventory[i] == -1) {
            g_louvreData.objectInventory[i] = (int)value;
            inserted = true;
            break;
        }
    }

    engine.setStateValue(cond, inserted ? "1.0" : "0.0");
    engine.setStateValue(notCond, inserted ? "0.0" : "1.0");
}

void plgAddCoffreObject(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgIsPresent(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string cond = args[1];
    std::string notCond = args[2];

    // TODO: implement correctly
    bool result = false;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (g_louvreData.objectInventory[i] == (int)value) {
            result = true;
            break;
        }
    }
    engine.setStateValue(cond, result ? "1.0" : "0.0");
    engine.setStateValue(notCond, result ? "0.0" : "1.0");
}

void plgRemoveObject(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (g_louvreData.objectInventory[i] == (int)value) {
            g_louvreData.objectInventory[i] = -1;
            break;
        }
    }
}

void plgStartTimer(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveTestSlot(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string cond = args[1];
    std::string notCond = args[2];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveLoad(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSaveCoffre(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveSetContextLabel(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string value = args[0];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgLoadSaveDrawSlot(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 4) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value1 = std::stod(args[0]);
    double value2 = std::stod(args[1]);
    double value3 = std::stod(args[2]);
    double value4 = std::stod(args[3]);

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgEnd(Engine& engine, std::vector<std::string> args)
{
    engine.end();
}

void plgInit2(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string variable = args[0];
    engine.setStateValue(variable, "0.0");
}

void plgInit(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string variable = args[1];

    bool ret = g_louvreData.arnVit.open("data/bdataheader.vit", "data/bdata1.arn");
    if (!ret) {
        LOG_ERROR("Failed to open ARN/VIT");
        return;
    }

    engine.setStateValue(variable, "0.0");
    if (value == 2) {
        engine.setStateValue(variable, "1.0");
    }
}

void plgLoadSaveEnterScript(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string var = args[0];
    std::string notVar = args[1];

    // TODO: implement
    engine.setStateValue(var, "0.0"); // Reloading
    engine.setStateValue(notVar, "1.0"); // Not reloading

    LOG_ERROR("Not implemented");
}

void plgLoadCoffre(Engine& engine, std::vector<std::string> args)
{
    // TODO: implement
    LOG_ERROR("Not implemented");
}

void plgSelect(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 3) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string w = args[1];
    std::string z = args[2];

    int objectId = g_louvreData.objectInventory[(int)value - 1];
    if (objectId == -1) {
        LOG_ERROR("Invalid object id");
        return;
    }

    g_louvreData.selectedObjectSlot = (int)value - 1;

    printPortefSelectedObject(engine, g_louvreData.selectedObjectSlot);
}

void plgDoAction(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 2) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    double value = std::stod(args[0]);
    std::string z = args[1];

    switch ((int)value) {
    case 1: {
        // Take
        if (g_louvreData.selectedObjectSlot == -1) {
            LOG_ERROR("No object selected");
            return;
        }

        int objectId = g_louvreData.objectInventory[g_louvreData.selectedObjectSlot];
        if (g_objectMap[objectId].canUse) {
            engine.setStateValue("inventaire", std::to_string((double)objectId));
        } else {
            LOG_ERROR("Object cannot be used");
        }
        break;
    }
    case 2:
        // See
        // TODO: implement
        break;
    case 4:
        // Combine
        // TODO: implement
        break;
    case 8:
        // Separate
        // TODO: implement
        break;
    default:
        LOG_ERROR("Invalid action");
        break;
    }
}

void plgDiscocier(Engine& engine, std::vector<std::string> args)
{
    if (args.size() != 1) {
        LOG_ERROR("Invalid number of arguments");
        return;
    }

    std::string z = args[0];

    // TODO: implement
    LOG_ERROR("Not implemented");
}

void registerPluginLouvre(Engine& engine)
{
    LOG_INFO("Registering plugin");

    engine.registerScriptPluginFunction("play_movie", &plgPlayMovie);
    engine.registerScriptPluginFunction("cmp", &plgCmp);
    engine.registerScriptPluginFunction("killtimer", &plgKillTimer);
    engine.registerScriptPluginFunction("cartedestination", &plgCarteDestination);
    engine.registerScriptPluginFunction("pausetimer", &plgPauseTimer);
    engine.registerScriptPluginFunction("add", &plgAdd);
    engine.registerScriptPluginFunction("initcoffre", &plgInitCoffre);
    engine.registerScriptPluginFunction("afficheportef", &plgAffichePortef);
    engine.registerScriptPluginFunction("afficheselection", &plgAfficheSelection);
    engine.registerScriptPluginFunction("affichecoffre", &plgAfficheCoffre);
    engine.registerScriptPluginFunction("scroll", &plgScroll);
    engine.registerScriptPluginFunction("selectportef", &plgSelectPorteF);
    engine.registerScriptPluginFunction("play_animbloc", &plgPlayAnimBloc);
    engine.registerScriptPluginFunction("until", &plgUntilLoop);
    engine.registerScriptPluginFunction("memoryrelease", &plgMemoryRelease);
    engine.registerScriptPluginFunction("selectcoffre", &plgSelectCoffre);
    engine.registerScriptPluginFunction("play_animbloc_number", &plgPlayAnimBlocNumber);
    engine.registerScriptPluginFunction("sub", &plgSub);
    engine.registerScriptPluginFunction("while", &plgWhileLoop);
    engine.registerScriptPluginFunction("ishere", &plgIsHere);
    engine.registerScriptPluginFunction("drawtextselection", &plgDrawTextSelection);
    engine.registerScriptPluginFunction("portefrollover", &plgPorteFRollover);
    engine.registerScriptPluginFunction("setmonde4", &plgSetMonde4);
    engine.registerScriptPluginFunction("getmonde4", &plgGetMonde4);
    engine.registerScriptPluginFunction("changecurseur", &plgChangeCurseur);
    engine.registerScriptPluginFunction("loadsave_context_restored", &plgLoadSaveContextRestored);
    engine.registerScriptPluginFunction("loadsave_capture_context", &plgLoadSaveCaptureContext);
    engine.registerScriptPluginFunction("loadsave_init_slots", &plgLoadSaveInitSlots);
    engine.registerScriptPluginFunction("loadsave_save", &plgLoadSaveSave);
    engine.registerScriptPluginFunction("multicd_set_next_script", &plgMultiCdSetNextScript);
    engine.registerScriptPluginFunction("reset", &plgReset);
    engine.registerScriptPluginFunction("addobject", &plgAddObject);
    engine.registerScriptPluginFunction("addcoffreobject", &plgAddCoffreObject);
    engine.registerScriptPluginFunction("ispresent", &plgIsPresent);
    engine.registerScriptPluginFunction("removeobject", &plgRemoveObject);
    engine.registerScriptPluginFunction("starttimer", &plgStartTimer);
    engine.registerScriptPluginFunction("loadsave_test_slot", &plgLoadSaveTestSlot);
    engine.registerScriptPluginFunction("loadsave_load", &plgLoadSaveLoad);
    engine.registerScriptPluginFunction("savecoffre", &plgSaveCoffre);
    engine.registerScriptPluginFunction("loadsave_set_context_label", &plgLoadSaveSetContextLabel);
    engine.registerScriptPluginFunction("loadsave_draw_slot", &plgLoadSaveDrawSlot);
    engine.registerScriptPluginFunction("end", &plgEnd);
    engine.registerScriptPluginFunction("init2", &plgInit2);
    engine.registerScriptPluginFunction("init", &plgInit);
    engine.registerScriptPluginFunction("loadsave_enter_script", &plgLoadSaveEnterScript);
    engine.registerScriptPluginFunction("loadcoffre", &plgLoadCoffre);
    engine.registerScriptPluginFunction("select", &plgSelect);
    engine.registerScriptPluginFunction("doaction", &plgDoAction);
    engine.registerScriptPluginFunction("discocier", &plgDiscocier);
}
