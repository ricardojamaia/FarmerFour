#include <Debug.h>

#include <TimeLib.h>    

#include <Controller.h>
#include <UserInterface.h>
#include <DS3232RTC.h>

#define UI_ACTION_SET_HOUR 30
#define UI_ACTION_SET_MINUTE 31
#define UI_ACTION_SET_DAY 32
#define UI_ACTION_SET_MONTH 33
#define UI_ACTION_SET_YEAR 34

#define UI_ACTION_SET_VALVE_0_ACTIVE    40
#define UI_ACTION_SET_VALVE_0_HOUR      41
#define UI_ACTION_SET_VALVE_0_MINUTE    42
#define UI_ACTION_SET_VALVE_0_REPEAT    43
#define UI_ACTION_SET_VALVE_0_DURATION  44

#define UI_ACTION_SET_VALVE_1_ACTIVE    50
#define UI_ACTION_SET_VALVE_1_HOUR      51
#define UI_ACTION_SET_VALVE_1_MINUTE    52
#define UI_ACTION_SET_VALVE_1_REPEAT    53
#define UI_ACTION_SET_VALVE_1_DURATION  54

#define UI_ACTION_SET_VALVE_2_ACTIVE    60
#define UI_ACTION_SET_VALVE_2_HOUR      61
#define UI_ACTION_SET_VALVE_2_MINUTE    62
#define UI_ACTION_SET_VALVE_2_REPEAT    63
#define UI_ACTION_SET_VALVE_2_DURATION  64

Controller &controller = Controller::getInstance();
UserInterface &userInterface = UserInterface::getInstance();


const uint32_t BAUD_RATE = 115200; // Serial Monitor speed setting

MD_Menu::value_t vBuf; // interface buffer for values
MD_Menu::value_t *_mnuIValueSetTime(MD_Menu::mnuId_t id, bool bGet);
MD_Menu::value_t *_mnuIValueSetValve(MD_Menu::mnuId_t id, bool bGet);

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
    {
        {00, "----- MENU -----", 10, 11, 0},
        {01, "PROGRAMAR REGA", 15, 17, 0},
        {02, "ACERTAR RELOGIO", 30, 34, 0},
        {05, "VALVULA 1", 40, 44, 0},
        {06, "VALVULA 2", 50, 54, 0},
        {07, "VALVULA 3", 60, 64, 0},
};

// Menu Items ----------
const PROGMEM MD_Menu::mnuItem_t mnuItm[] =
    {
        // Starting (Root) menu
        {10, "PROGRAMAR REGA", MD_Menu::MNU_MENU, 01},
        {11, "RELOGIO", MD_Menu::MNU_MENU, 02},

        // PROGRAMAR REGA menu
        {15, "VALVULA 1", MD_Menu::MNU_MENU, 05},
        {16, "VALVULA 2", MD_Menu::MNU_MENU, 06},
        {17, "VALVULA 3", MD_Menu::MNU_MENU, 07},

        // Input Data submenu
        {30, "HORAS", MD_Menu::MNU_INPUT, UI_ACTION_SET_HOUR},
        {31, "MINUTOS", MD_Menu::MNU_INPUT, UI_ACTION_SET_MINUTE},
        {32, "DIA", MD_Menu::MNU_INPUT, UI_ACTION_SET_DAY},
        {33, "MES", MD_Menu::MNU_INPUT, UI_ACTION_SET_MONTH},
        {34, "ANO", MD_Menu::MNU_INPUT, UI_ACTION_SET_YEAR},

        {40, "ACTIVAR", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_0_ACTIVE},
        {41, "HORA", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_0_HOUR},
        {42, "MINUTO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_0_MINUTE},
        {43, "PERIODO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_0_REPEAT},
        {44, "DURACAO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_0_DURATION},

        {50, "ACTIVAR", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_1_ACTIVE},
        {51, "HORA", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_1_HOUR},
        {52, "MINUTO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_1_MINUTE},
        {53, "PERIODO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_1_REPEAT},
        {54, "DURACAO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_1_DURATION},

        {60, "ACTIVAR", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_2_ACTIVE},
        {61, "HORA", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_2_HOUR},
        {62, "MINUTO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_2_MINUTE},
        {63, "PERIODO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_2_REPEAT},
        {64, "DURACAO", MD_Menu::MNU_INPUT, UI_ACTION_SET_VALVE_2_DURATION},
};

const PROGMEM char repeatOptionsList[] = "12 HORAS|1 DIA|2 DIAS|4 DIAS|1 SEMANA";
const int repeatOptionsValues[] = {12, 24, 48, 96, 168};
#define FF_NUM_REPEAT_OPTIONS 5

// Input Items ---------
const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
    {
        {UI_ACTION_SET_HOUR, "HORAS", MD_Menu::INP_INT, _mnuIValueSetTime, 2, 0, 0, 23, 0, 10, nullptr},
        {UI_ACTION_SET_MINUTE, "MINUTOS", MD_Menu::INP_INT, _mnuIValueSetTime, 2, 0, 0, 59, 0, 10, nullptr},
        {UI_ACTION_SET_DAY, "DIA", MD_Menu::INP_INT, _mnuIValueSetTime, 2, 1, 0, 31, 0, 10, nullptr},
        {UI_ACTION_SET_MONTH, "MES", MD_Menu::INP_INT, _mnuIValueSetTime, 2, 1, 0, 12, 0, 10, nullptr},
        {UI_ACTION_SET_YEAR, "ANO", MD_Menu::INP_INT, _mnuIValueSetTime, 4, 2020, 0, 2050, 0, 10, nullptr},

        {UI_ACTION_SET_VALVE_0_ACTIVE, "ACTIVAR", MD_Menu::INP_BOOL, _mnuIValueSetValve, 1, 0, 0, 0, 0, 0, nullptr},
        {UI_ACTION_SET_VALVE_0_HOUR, "HORA", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 23, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_0_MINUTE, "MINUTO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_0_REPEAT, "PER", MD_Menu::INP_LIST, _mnuIValueSetValve, 8, 0, 0, 0, 0, 0, repeatOptionsList},
        {UI_ACTION_SET_VALVE_0_DURATION, "DURACAO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},

        {UI_ACTION_SET_VALVE_1_ACTIVE, "ACTIVAR", MD_Menu::INP_BOOL, _mnuIValueSetValve, 1, 0, 0, 0, 0, 0, nullptr},
        {UI_ACTION_SET_VALVE_1_HOUR, "HORA", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 23, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_1_MINUTE, "MINUTO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_1_REPEAT, "PER", MD_Menu::INP_LIST, _mnuIValueSetValve, 8, 0, 0, 0, 0, 0, repeatOptionsList},
        {UI_ACTION_SET_VALVE_1_DURATION, "DURACAO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},

        {UI_ACTION_SET_VALVE_2_ACTIVE, "ACTIVAR", MD_Menu::INP_BOOL, _mnuIValueSetValve, 1, 0, 0, 0, 0, 0, nullptr},
        {UI_ACTION_SET_VALVE_2_HOUR, "HORA", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_2_MINUTE, "MINUTO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},
        {UI_ACTION_SET_VALVE_2_REPEAT, "PER", MD_Menu::INP_LIST, _mnuIValueSetValve, 8, 0, 0, 0, 0, 0, repeatOptionsList},
        {UI_ACTION_SET_VALVE_2_DURATION, "DURACAO", MD_Menu::INP_INT, _mnuIValueSetValve, 2, 0, 0, 59, 0, 10, nullptr},
};

// bring it all together in the global menu object
MD_Menu M(UserInterface::_navigation, UserInterface::_display, // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr),                          // menu header data
          mnuItm, ARRAY_SIZE(mnuItm),                          // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));                         // menu input data

MD_Menu::value_t *_mnuIValueSetTime(MD_Menu::mnuId_t id, bool bGet)
// Value request callback for integers variables
{
    MD_Menu::value_t *r = &vBuf;
    //rtclib::DateTime dateTime = rtc.now();
    tmElements_t t;
    breakTime(now(), t);

    switch (id)
    {
    case UI_ACTION_SET_HOUR:
        if (bGet)
            vBuf.value = hour();
        else
        {
            t.Hour = vBuf.value;
            DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Hour value changed to "));
            DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
        }
        break;
    case UI_ACTION_SET_MINUTE:
        if (bGet)
        {
            vBuf.value = minute();
        }
        else
        {
            t.Minute = vBuf.value;
            DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Minute value changed to "));
            DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
        }
        break;
    case UI_ACTION_SET_DAY:
        if (bGet)
        {
            t.Day = vBuf.value;
        }
        else
        {
            t.Day = vBuf.value;
            DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Day value changed to "));
            DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
        }
        break;
    case UI_ACTION_SET_MONTH:
        if (bGet)
        {
            vBuf.value = month();
        }
        else
        {
            t.Month = vBuf.value;
            DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Month value changed to "));
            DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
        }
        break;
    case UI_ACTION_SET_YEAR:
        if (bGet)
        {
            vBuf.value = year();
        }
        else
        {
            t.Year = vBuf.value - 1970;
            DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Year value changed to "));
            DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
        }
        break;

    default:
        r = nullptr;
        break;
    }

    if(!bGet){
        time_t newTime = makeTime(t);
        setTime(newTime);
        RTC.set(newTime);
    }

    //rtc.adjust(dateTime);
    return (r);
}

MD_Menu::value_t *_mnuIValueSetValve(MD_Menu::mnuId_t id, bool bGet)
{
    MD_Menu::value_t *r = &vBuf;
    int valveId = 0;

    bool restart_nedded = false;

    switch (id)
    {
        case UI_ACTION_SET_VALVE_0_ACTIVE:
        case UI_ACTION_SET_VALVE_1_ACTIVE:
        case UI_ACTION_SET_VALVE_2_ACTIVE:
            valveId = (id - UI_ACTION_SET_VALVE_0_ACTIVE) / 10;
            if (bGet)
                vBuf.value = controller.isActive(valveId);
            else
            {
                if (vBuf.value){
                    controller.enable(valveId);
                } else {
                    controller.disable(valveId);
                }
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Valve "));
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, valveId);
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(" is now "));
                DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, controller.isActive(valveId) ? F("active") : F("inactive"));
            }
            break;
        case UI_ACTION_SET_VALVE_0_HOUR:
        case UI_ACTION_SET_VALVE_1_HOUR:
        case UI_ACTION_SET_VALVE_2_HOUR:
            valveId = (id - UI_ACTION_SET_VALVE_0_HOUR) / 10;
            if (bGet)
                vBuf.value = controller.getHour(valveId);
            else
            {
                controller.setHour(valveId, vBuf.value);
                restart_nedded = true;
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Valve "));
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, valveId);
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(" start hour set to "));
                DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
            }
            break;
        case UI_ACTION_SET_VALVE_0_MINUTE:
        case UI_ACTION_SET_VALVE_1_MINUTE:
        case UI_ACTION_SET_VALVE_2_MINUTE:
            valveId = (id - UI_ACTION_SET_VALVE_0_MINUTE) / 10;
            if (bGet)
                vBuf.value = controller.getMinute(valveId);
            else
            {
                controller.setMinute(valveId, vBuf.value);
                restart_nedded = true;
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Valve "));
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, valveId);
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(" start minute set to "));
                DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
            }
            break;
        case UI_ACTION_SET_VALVE_0_DURATION:
        case UI_ACTION_SET_VALVE_1_DURATION:
        case UI_ACTION_SET_VALVE_2_DURATION:
            valveId = (id - UI_ACTION_SET_VALVE_0_DURATION) / 10;
            if (bGet)
                vBuf.value = controller.getDuration(valveId);
            else
            {
                controller.setDuration(valveId, vBuf.value);
                restart_nedded = true;
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Valve "));
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, valveId);
                DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(" duration set to "));
                DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, vBuf.value);
            }
            break;
        case UI_ACTION_SET_VALVE_0_REPEAT:
        case UI_ACTION_SET_VALVE_1_REPEAT:
        case UI_ACTION_SET_VALVE_2_REPEAT:
            valveId = (id - UI_ACTION_SET_VALVE_0_REPEAT) / 10;
            if (bGet){
                int repeatHours = controller.getRepeat(valveId);
                int i = 0;

                for (i = 0; i < FF_NUM_REPEAT_OPTIONS; i++)
                {
                    if (repeatHours == repeatOptionsValues[i]){
                        break;
                    }
                }
                vBuf.value = i;
            }
            else
            {
                int i = vBuf.value;
                if ((i >= 0) || (i < FF_NUM_REPEAT_OPTIONS)){
                    controller.setRepeat(valveId, repeatOptionsValues[i]);
                    restart_nedded = true;
                    DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F("Valve "));
                    DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, valveId);
                    DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(" repeat set to "));
                    DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, i);
                    DPRINT(DEBUG_FF_MAIN, DEBUG_HIGH, F(": "));
                    DPRINTLN(DEBUG_FF_MAIN, DEBUG_HIGH, repeatOptionsValues[i]);
                }
            }
            break;
        default:
            r = nullptr;
            break;
    }

    if (!bGet){
        controller.save();
    }

    if (restart_nedded && controller.isActive(valveId)){
        controller.disable(valveId);
        controller.enable(valveId);  
    }


    return (r);
}


void switchCallback(int switch_id, bool isOn){
    DPRINT(DEBUG_FF_MAIN, DEBUG_LOW, "Switch ");
    DPRINT(DEBUG_FF_MAIN, DEBUG_LOW, switch_id);
    DPRINTLN(DEBUG_FF_MAIN, DEBUG_LOW, isOn ? " is On.": " is Off.");
    if (isOn){
        controller.turnOnValve(switch_id);
    } else {
        controller.turnOffValve(switch_id);
    }
}

void setup(void)
{
    Serial.begin(BAUD_RATE);

    // Get the time from the RTC
    setSyncProvider(RTC.get);
    if (timeStatus() != timeSet){
        DPRINTLN(DEBUG_FF_MAIN, DEBUG_LOW, "Unable to sync with the RTC");
    } else {
        DPRINTLN(DEBUG_FF_MAIN, DEBUG_LOW,"System time set by RTC.");
    }
        

    controller.setup();
    controller.load();
    userInterface.begin(&M);

    userInterface.setCallback(&switchCallback);

}

void loop(void)
{
    userInterface.loop();
    Alarm.delay(0);
}
