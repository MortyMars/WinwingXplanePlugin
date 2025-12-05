#include "ff767-fcu-efis-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-fcu-efis.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>

FF767FCUEfisProfile::FF767FCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    
    Dataref::getInstance()->monitorExistingDataref<float>("lights/glareshield1_rhe",                            //OK????
                                                          [product](float brightness) {
        bool hasPower = Dataref::getInstance()->getCached<bool>("sim/cockpit2/autopilot/autopilot_has_power");  //OK
        uint8_t target = hasPower ? brightness * 255 : 0;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EXPED_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);

        uint8_t screenBrightness = hasPower ? 200 : 0;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);

        uint8_t ledBrightness = 255;
        product->setLedBrightness(FCUEfisLed::OVERALL_GREEN, hasPower ? ledBrightness : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_OVERALL_GREEN, hasPower ? ledBrightness : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_OVERALL_GREEN, hasPower ? ledBrightness : 0);

        product->forceStateSync();
    });

    // We abuse the GPU dataref to trigger an update when the UI is closed.
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/electrical/gpuAvailable",                           //OK????
                                                         [product](bool gpuHatchOpen) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/autopilot/autopilot_has_power");    //OK
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit2/autopilot/autopilot_has_power",              //OK
                                                         [product](bool hasPower) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("lights/glareshield1_rhe");                       //OK????
    });

   // OK ========== LEDs =================================================================================================
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/AP/lnavButton", [this, product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/AP/vnavButton", [this, product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/eprButton", [this, product](bool armed) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/locButton", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/appButton", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/cmd_L_Button", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/cmd_C_Button", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/cmd_R_Button", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/1/map4", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/2/map4", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/1/map5", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/2/map5", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/1/map2", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/2/map2", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    // ARPT crash X-PLANE - Revoir affectation 'map3' ?
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/1/map3", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_ARPT_GREEN, armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/ctrlPanel/2/map3", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, armed || isTestMode() ? 1 : 0);
    });
    
    /*Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/desengageLever", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, (armed ? 0 : 1) || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/desengageLever", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, (armed ? 0 : 1) || isTestMode() ? 1 : 0);
    });*/
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/desengageLever", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, !armed || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/AP/desengageLever", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, !armed || isTestMode() ? 1 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/AP/fd1Switcher", [this, product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/AP/fd2Switcher", [this, product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
    // ========== Fin LEDs ============================================================================================
    
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/cptCAUTION",
                                                         [this, product](bool isCaution) {
        bool isWarning = Dataref::getInstance()->getCached<bool>("1-sim/ckpt/lampsGlow/cptWARNING");
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN,  isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN,  isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_ARPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/foCAUTION",
                                                         [this, product](bool isCaution) {
        bool isWarning = Dataref::getInstance()->getCached<bool>("1-sim/ckpt/lampsGlow/foWARNING");
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN,  isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN,  isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/cptWARNING",
                                                         [this, product](bool on) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/warnings/annunciators/master_caution");  //OK????
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/foWARNING",
                                                         [this, product](bool on) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/warnings/annunciators/master_caution");  //OK????
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/testPanel/test1Button",      //OK????
                                                        [this, product](int isTest) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/autopilot/autopilot_has_power");    //OK
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpCaptAP");
        
        //OK
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/lnavButton");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/vnavButton");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/eprButton");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/locButton");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/appButton");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/cmd_L_Button");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/cmd_C_Button");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/AP/cmd_R_Button");
        //finOK
        
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/warnings/annunciators/master_warning");  //OK????
        //Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/foWARNING");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/comm/AP/ap_disc"); //OK
    });

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/efis/isBaroStdL",                      //OK
                                                          [this, product](float animValue) {
        AppState::getInstance()->executeAfterDebounced("cptStdChanged", 50, [this, product]() {
            isStdCaptain = !isStdCaptain;

            float baroValue = Dataref::getInstance()->get<float>("1-sim/gauges/baroINHg1_left");        //OK
            if (isStdCaptain && fabs(baroValue - 29.92f) > std::numeric_limits<float>::epsilon()) {
                isStdCaptain = false;
            }

            product->updateDisplays();
        });
    });

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/efis/isBaroStdR",                      //OK
                                                          [this, product](float animValue) {
        AppState::getInstance()->executeAfterDebounced("foStdChanged", 50, [this, product]() {
            isStdFirstOfficer = !isStdFirstOfficer;

            float baroValue = Dataref::getInstance()->get<float>("1-sim/gauges/baroINHg1_right");       //OK
            if (isStdFirstOfficer && fabs(baroValue - 29.92f) > std::numeric_limits<float>::epsilon()) {
                isStdFirstOfficer = false;
            }

            product->updateDisplays();
        });
    });
}

// Destructeur ====================================================================================
FF767FCUEfisProfile::~FF767FCUEfisProfile() {
    Dataref::getInstance()->unbind("lights/glareshield1_rhe");                      //OK????
    Dataref::getInstance()->unbind("1-sim/electrical/gpuAvailable");                //OK????
    Dataref::getInstance()->unbind("sim/cockpit2/autopilot/autopilot_has_power");   //OK
    
    // OK
    Dataref::getInstance()->unbind("1-sim/AP/lnavButton");
    Dataref::getInstance()->unbind("1-sim/AP/vnavButton");
    Dataref::getInstance()->unbind("1-sim/AP/eprButton");
    Dataref::getInstance()->unbind("1-sim/AP/locButton");
    Dataref::getInstance()->unbind("1-sim/AP/appButton");
    Dataref::getInstance()->unbind("1-sim/AP/cmd_L_Button");
    Dataref::getInstance()->unbind("1-sim/AP/cmd_C_Button");
    Dataref::getInstance()->unbind("1-sim/AP/cmd_R_Button");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/1/map2");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/2/map2");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/1/map3");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/2/map3");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/1/map4");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/2/map4");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/1/map5");
    Dataref::getInstance()->unbind("1-sim/efis/ctrlPanel/2/map5");
    // fin OK
    
    //Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/cptCAUTION");
    //Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/foCAUTION");
    Dataref::getInstance()->unbind("sim/cockpit/warnings/annunciators/master_caution"); //OK????
    Dataref::getInstance()->unbind("sim/cockpit/warnings/annunciators/master_warning"); //OK????
}


// OK ================================================================================================
bool FF767FCUEfisProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/AP/cmd_C_Button") ||
           Dataref::getInstance()->exists("1-sim/efis/ctrlPanel/2/map3") ||
           Dataref::getInstance()->exists("1-sim/ndpanel/2/hsiRangeRotary");
}


// OK Datarefs =======================================================================================
const std::vector<std::string> &FF767FCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        
        // MCP - Power
        "sim/cockpit2/autopilot/autopilot_has_power",                   //OK

        // MCP - Speed
        "1-sim/AP/iasmach",                                             //OK
        //"777/autopilot/speed_mode", // SPD, FLCH, etc.
        "1-sim/AP/dig3/spdSetting",                                     //OK
        "1-sim/output/mcp/fma_spd_mode",

        // MCP - Heading
        // "1-sim/output/mcp/isHdgTrg",
        "1-sim/AP/hdgConfButton",                                       //OK??
        "1-sim/AP/hdgSetting",                                          //OK
        "1-sim/output/mcp/fma_hdg_mode",

        // MCP - Altitude
        "1-sim/AP/dig5/altSetting",                                     //OK
        "1-sim/output/mcp/fma_alt_mode",

        // MCP - Vertical Speed
        "1-sim/AP/vviSetting",                                          //OK
        "1-sim/output/mcp/fma_vs_mode",

        // EFIS - Barometric settings
        "1-sim/gauges/baroINHG1_left",                                  //OK
        "1-sim/gauges/baroINHG1_right",                                 //OK
        "1-sim/gauges/baroHPa1_left",                                   //OK
        "1-sim/gauges/baroHPa1_right",                                  //OK
        
        "1-sim/efis/isBaroHpaL", // 0=inHg,1=hPa                        //OK
        "1-sim/efis/isBaroHpaR",                                        //OK

        // ND Mode and Range
        "1-sim/efis/ctrlPanel/1/hsiModeRotary", // 0=APP,1=VOR,2=MAP,3=PLAN OK
        "1-sim/efis/ctrlPanel/2/hsiModeRotary",                           //OK
        "1-sim/ndpanel/1/hsiRangeRotary",       // 10,20,40,80,160,320,640  OK
        "1-sim/ndpanel/2/hsiRangeRotary",                                 //OK

        "1-sim/testPanel/test1Button",                                  //OK????
    };

    return datarefs;
}


// OK Association Commandes / Boutons ================================================================
const std::unordered_map <uint16_t, FCUEfisButtonDef> &FF767FCUEfisProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, FCUEfisButtonDef> buttons = {

        // MCP --------------------------------------------------------------------------
        {0, {"SPD",     "1-sim/command/AP/iasmach_button"}},            //OK
        {1, {"LOC",     "1-sim/comm/AP/locButton"}},                    //OK
        // {2, {"HDG/TRK", "1-sim/command/mcpHdgTrkButton_button"}},    // don't exist on B767
        {3, {"AP1",     "1-sim/comm/AP/lnavButton"}},                   //OK
        {4, {"AP2",     "1-sim/comm/AP/vnavButton"}},                   //OK
        {5, {"A/THR",   "1-sim/command/AP/atSwitcher_trigger"}},        //OK
        {6, {"EXPED",   "1-sim/command/AP/cmd_L_Button_button"}},       //OK
        // {7, {"VS/FPA",  "1-sim/command/mcpVsFpaButton_button"}},     // don't exist on B767
        {8, {"APP",     "1-sim/comm/AP/appButton"}},                    //OK

        // Rotary encoders - Speed
        {9,  {"SPD DEC",  "1-sim/comm/AP/spdDN"}},                      //OK
        {10, {"SPD INC",  "1-sim/comm/AP/spdUP"}},                      //OK
        {11, {"SPD",      "1-sim/command/AP/spdConfButton_button"}},    //OK
        //{12, },

        // Rotary encoders - Heading
        {13, {"HDG DEC",  "1-sim/comm/AP/hdgDN"}},                      //OK
        {14, {"HDG INC",  "1-sim/comm/AP/hdgUP"}},                      //OK
        {15, {"HDG PUSH", "1-sim/command/AP/hdgConfButton_button"}},    //OK
        {16, {"HDG HOLD", "1-sim/comm/AP/hdgHoldButton"}},              //OK

        // Rotary encoders - Altitude
        {17, {"ALT DEC",  "1-sim/comm/AP/altDN"}},                      //OK
        {18, {"ALT INC",  "1-sim/comm/AP/altUP"}},                      //OK
        // {19, {"ALT PUSH", "1-sim/command/mcpAltRotary_push"}},       // don't exist on B767
        {20, {"ALT HOLD", "1-sim/comm/AP/altHoldButton"}},              //OK

        // Rotary encoders - Vertical Speed
        {21, {"VS DEC",   "1-sim/comm/AP/vviDN"}},                      //OK
        {22, {"VS INC",   "1-sim/comm/AP/vviUP"}},                      //OK
        {23, {"V/S",      "1-sim/comm/AP/vviButton"}},                  //OK
        //{24, },

        // Altitude par 100/1000
        // {25, {"ALT 100",  "1-sim/command/mcpAltModeSwitch_set_0"}},  // don't exist on B767
        // {26, {"ALT 1000", "1-sim/command/mcpAltModeSwitch_set_1"}},  // don't exist on B767

        
        // EFIS CAPT --------------------------------------------------------------------
        {32, {"L_FD",     "1-sim/command/AP/fd1Switcher_trigger"}},                 //OK
        {33, {"AP DISC",  "1-sim/comm/AP/ap_disc"}},                                //OK

        // ND Options
        {34, {"L_DATA",   "1-sim/command/ckpt/cptHsiDataButton/anim_pushbutton"}},  //OK
        {35, {"L_WPT",    "1-sim/command/ckpt/cptHsiWptButton/anim_pushbutton"}},   //OK
        {36, {"L_STA",    "1-sim/command/ckpt/cptHsiStaButton/anim_pushbutton"}},   //OK
        //{37, },
        {38, {"L_ARPT",   "1-sim/command/ckpt/cptHsiArptButton/anim_pushbutton"}},  //OK

        // BARO
        {39, {"L_BARO PUSH", "1-sim/efis/isBaroStdL",FCUEfisDatarefType::SET_VALUE, 1.0}},  //Trough Dataref
        {40, {"R_BARO PULL", "1-sim/efis/isBaroStdL",FCUEfisDatarefType::SET_VALUE, 0.0}},  //Trough Dataref
        {41, {"L_BARO DEC",  "1-sim/command/gauges/baroRotary_left_rotary-"}},              //OK
        {42, {"L_BARO INC",  "1-sim/command/gauges/baroRotary_left_rotary+"}},              //OK
        {43, {"L_inHg",      "1-sim/efis/isBaroHpaL",FCUEfisDatarefType::SET_VALUE, 0.0}},  //Trough Dataref
        {44, {"L_hPa",       "1-sim/efis/isBaroHpaL",FCUEfisDatarefType::SET_VALUE, 1.0}},  //Trough Dataref

        // ND Mode selector - Commands not found —> go through Datarefs
        {45, {"L_MODE APP",  "1-sim/efis/ctrlPanel/1/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {46, {"L_MODE VOR",  "1-sim/efis/ctrlPanel/1/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 1.0}},
        //{47, },
        {48, {"L_MODE MAP",  "1-sim/efis/ctrlPanel/1/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 2.0}},
        {49, {"L_MODE PLAN", "1-sim/efis/ctrlPanel/1/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 3.0}},

        // ND Range selector - Commands not found —> go through Datarefs
        {50, {"L_RANGE 10",  "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 10.0}},
        {51, {"L_RANGE 20",  "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 20.0}},
        {52, {"L_RANGE 40",  "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 40.0}},
        {53, {"L_RANGE 80",  "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 80.0}},
        {54, {"L_RANGE 160", "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 160.0}},
        {55, {"L_RANGE 320", "1-sim/ndpanel/1/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 320.0}},

        // VOR/ADF selectors - Commands not found —> go through Datarefs
        {56, {"L_VORL VOR",  "1-sim/ckpt/cptHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, 1.0}},
        {57, {"L_VORL OFF",  "1-sim/ckpt/cptHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {58, {"L_VORL ADF",  "1-sim/ckpt/cptHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, -1.0}},
        {59, {"L_VORR VOR",  "1-sim/ckpt/cptHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, 1.0}},
        {60, {"L_VORR OFF",  "1-sim/ckpt/cptHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {61, {"L_VORR ADF",  "1-sim/ckpt/cptHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, -1.0}},
        //{62, },
        //{63, },

        
        // EFIS FO ----------------------------------------------------------------------
        {64, {"R_FD",     "1-sim/command/AP/fd2Switcher_trigger"}},                 //OK
        {65, {"AP DISC",  "1-sim/comm/AP/ap_disc"}},                                //OK

        // ND Options
        {66, {"R_DATA",   "1-sim/command/ckpt/foHsiDataButton/anim_pushbutton"}},   //OK
        {67, {"R_WPT",    "1-sim/command/ckpt/foHsiWptButton/anim_pushbutton"}},    //OK
        {68, {"R_STA",    "1-sim/command/ckpt/foHsiStaButton/anim_pushbutton"}},    //OK
        //{69, },
        {70, {"R_ARPT",   "1-sim/command/ckpt/foHsiArptButton/anim_pushbutton"}},   //OK

        // BARO
        {71, {"R_BARO PUSH", "1-sim/efis/isBaroStdR",FCUEfisDatarefType::SET_VALUE, 1.0}},  //Trough Dataref
        {72, {"R_BARO PULL", "1-sim/efis/isBaroStdR",FCUEfisDatarefType::SET_VALUE, 0.0}},  //Trough Dataref
        {73, {"R_BARO DEC",  "1-sim/command/gauges/baroRotary_right_rotary-"}},             //OK
        {74, {"R_BARO INC",  "1-sim/command/gauges/baroRotary_right_rotary+"}},             //OK
        {75, {"R_inHg",      "1-sim/efis/isBaroHpaR",FCUEfisDatarefType::SET_VALUE, 0.0}},  //Trough Dataref
        {76, {"R_hPa",       "1-sim/efis/isBaroHpaR",FCUEfisDatarefType::SET_VALUE, 1.0}},  //Trough Dataref

        // ND Mode selector - Commands not found —> go through Datarefs
        {77, {"R_MODE APP",  "1-sim/efis/ctrlPanel/2/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {78, {"R_MODE VOR",  "1-sim/efis/ctrlPanel/2/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 1.0}},
        //{79, },
        {80, {"R_MODE MAP",  "1-sim/efis/ctrlPanel/2/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 2.0}},
        {81, {"R_MODE PLAN", "1-sim/efis/ctrlPanel/2/hsiModeRotary",FCUEfisDatarefType::SET_VALUE, 3.0}},

        // ND Range selector - Command not found —> go through Dataref
        {82, {"R_RANGE 10",  "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 10.0}},
        {83, {"R_RANGE 20",  "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 20.0}},
        {84, {"R_RANGE 40",  "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 40.0}},
        {85, {"R_RANGE 80",  "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 80.0}},
        {86, {"R_RANGE 160", "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 160.0}},
        {87, {"R_RANGE 320", "1-sim/ndpanel/2/hsiRangeRotary",FCUEfisDatarefType::SET_VALUE, 320.0}},

        // VOR/ADF selectors - Command not found —> go through Dataref
        {88, {"R_VORL VOR",  "1-sim/ckpt/foHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, 1.0}},
        {89, {"R_VORL OFF",  "1-sim/ckpt/foHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {90, {"R_VORL ADF",  "1-sim/ckpt/foHsiVorLSwitch/anim",FCUEfisDatarefType::SET_VALUE, -1.0}},
        {91, {"R_VORR VOR",  "1-sim/ckpt/foHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, 1.0}},
        {92, {"R_VORR OFF",  "1-sim/ckpt/foHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, 0.0}},
        {93, {"R_VORR ADF",  "1-sim/ckpt/foHsiVorRSwitch/anim",FCUEfisDatarefType::SET_VALUE, -1.0}},

    };
    return buttons;
}


// OK ================================================================================================
void FF767FCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();

    data.displayEnabled = datarefManager->getCached<bool>("sim/cockpit2/autopilot/autopilot_has_power"); //OK
    data.displayTest = isTestMode();

    // SPD ------------------------------------------------------------------------------
    data.spdMach = datarefManager->getCached<bool>("1-sim/AP/iasmach");         //OK
    float speed = datarefManager->getCached<float>("1-sim/AP/dig3/spdSetting"); //OK

    if (speed > 0) {
        std::stringstream ss;
        if (data.spdMach) {
            int machHundredths = static_cast<int>(std::round(speed * 100));
            ss << std::setfill('0') << std::setw(3) << machHundredths;
        } else {
            ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
        }
        data.speed = ss.str();
    } else {
        data.speed = "---";
    }

    data.spdManaged = false;

    // HDG ------------------------------------------------------------------------------
    float heading = datarefManager->getCached<float>("1-sim/AP/hdgSetting");            //OK
    if (heading >= 0) {
        int hdgDisplay = static_cast<int>(heading) % 360;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << hdgDisplay;
        data.heading = ss.str();
    } else {
        data.heading = "---";
    }

    data.hdgManaged = false;
    data.hdgTrk = datarefManager->getCached<bool>("1-sim/AP/hdgConfButton") == false;   //OK

    // ALT ------------------------------------------------------------------------------
    float altitude = datarefManager->getCached<float>("1-sim/AP/dig5/altSetting");      //OK
    if (altitude >= 0) {
        int altInt = static_cast<int>(altitude);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << altInt;
        data.altitude = ss.str();
    } else {
        data.altitude = "-----";
    }

    data.altManaged = false;

    // VS -------------------------------------------------------------------------------
    float vs = datarefManager->getCached<float>("1-sim/AP/vviSetting");                 //OK

    data.vsMode = true;
    data.fpaMode = false;

    std::stringstream ss;
    int vsInt = static_cast<int>(std::round(vs));
    int absVs = std::abs(vsInt);

    ss << std::setfill('0') << std::setw(4) << absVs;
    data.verticalSpeed = ss.str();

    data.vsSign = (vs >= 0);
    data.fpaComma = false;
    data.vsIndication = true;
    data.fpaIndication = false;
    data.vsVerticalLine = true;

    data.latMode = true;

    for (int i = 0; i < 2; i++) {
        bool isCaptain = i == 0;

        bool isBaroHpa = datarefManager->getCached<bool>(isCaptain ? "1-sim/efis/isBaroHpaL" : "1-sim/efis/isBaroHpaR"); // OK
        float baroValue = datarefManager->getCached<float>(isCaptain ? "1-sim/gauges/baroINHg1_left" : "1-sim/gauges/baroINHg1_right"); // OK

        EfisDisplayValue value = {
            .displayEnabled = data.displayEnabled,
            .displayTest = data.displayTest,
            .baro = "",
            .unitIsInHg = false,
            .isStd = (isCaptain && isStdCaptain) || (!isCaptain && isStdFirstOfficer),
        };

        if (!value.isStd && baroValue > 0) {
            value.setBaro(baroValue, !isBaroHpa);
        }

        if (isCaptain) {
            data.efisLeft = value;
        } else {
            data.efisRight = value;
        }
    }
}


// ================================================================================================
void FF767FCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);
    }

    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE) {
        int currentValue = datarefManager->get<int>(button->dataref.c_str());
        int newValue = currentValue ? 0 : 1;
        datarefManager->set<int>(button->dataref.c_str(), newValue);
    }

    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}


// ================================================================================================
bool FF767FCUEfisProfile::isTestMode() {
    return Dataref::getInstance()->get<int>("1-sim/testPanel/test1Button") == 2; //OK????
}
