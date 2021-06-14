//
// Created by Ricardo Maia  on 12/06/2020.
//

#include <TimeLib.h>
#include <UserInterface.h>
#include <Debug.h>

static LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// LCD Shield keypad -----------------------

#define LCD_KEYS_PIN A0

// These key values work for most LCD shields
MD_UISwitch_Analog::uiAnalogKeys_t keyTable[] =
    {
        {15, 15, 'R'},  // Right
        {99, 15, 'U'},  // Up
        {255, 15, 'D'}, // Down
        {408, 15, 'L'}, // Left
        {639, 15, 'S'}, // Select
};

MD_UISwitch_Analog lcdKeys(LCD_KEYS_PIN, keyTable, ARRAY_SIZE(keyTable));

// Menu Setup -----------------------------
const bool AUTO_START = true; // auto start the menu, manual detect and start if false

const uint16_t MENU_TIMEOUT = 8000; // in milliseconds

bool UserInterface::_display(MD_Menu::userDisplayAction_t action, char *msg)
{
    return UserInterface::getInstance().display(action, msg);
}

bool UserInterface::display(MD_Menu::userDisplayAction_t action, char *msg)
{
    static char szLine[LCD_COLS + 1] = {'\0'};

    switch (action)
    {
    case MD_Menu::DISP_INIT:
        lcd.begin(LCD_COLS, LCD_ROWS);
        lcd.clear();
        lcd.noCursor();
        memset(szLine, ' ', LCD_COLS);
        break;

    case MD_Menu::DISP_CLEAR:
        lcd.clear();
        break;

    case MD_Menu::DISP_L0:
        lcd.print("----- MENU -----");
        break;

    case MD_Menu::DISP_L1:
        lcd.setCursor(0, 1);
        lcd.print(szLine);
        lcd.setCursor(0, 1);
        lcd.print(msg);
        break;
    }

    return (true);
}

MD_Menu::userNavAction_t UserInterface::_navigation(uint16_t &incDelta)
{
    return UserInterface::getInstance().navigation(incDelta);
}

MD_Menu::userNavAction_t UserInterface::navigation(uint16_t &incDelta)
// Using switches found on a LCD shield
// up and down map to INC and DEC
// Right and Left map to ESC
// Select maps to SEL
{
    incDelta = 1;

    MD_UISwitch::keyResult_t k = lcdKeys.read();

    if (k == MD_UISwitch::KEY_UP)
    {
        switch (lcdKeys.getKey())
        {
        case 'D':
            return (MD_Menu::NAV_DEC);
        case 'U':
            return (MD_Menu::NAV_INC);
        case 'S':
            return (MD_Menu::NAV_SEL);
        case 'R':
        case 'L':
            return (MD_Menu::NAV_ESC);
        }
    }

    return (MD_Menu::NAV_NULL);
}

void UserInterface::begin(MD_Menu *menu)
{
    M = menu;
    backlightOn = true;
    display(MD_Menu::DISP_INIT);

    digitalWrite(LCD_BL, LOW);
    pinMode(LCD_BL, INPUT);

    lcdKeys.begin();
    lcdKeys.enableRepeat(false);
    lcdKeys.enableLongPress(false);
    lcdKeys.enableDoublePress(false);
    lcdKeys.setPressTime(400);
    

    M->begin();
    M->setMenuWrap(true);
    M->setAutoStart(AUTO_START);
    M->setTimeout(MENU_TIMEOUT);

    for(int i = 0; i < NUM_SWITCHES; i++){
        pinMode(m_switches_pins[i], INPUT_PULLUP);
        pinMode(m_gnd_pins[i], OUTPUT);
        digitalWrite(m_gnd_pins[i], LOW);

        m_switches_last_state[i] = digitalRead(m_switches_pins[i]);
    }
}

void UserInterface::loop()
{
    static time_t prevDisplayTime = 0;
    // start with the background light on
    static time_t lastMenuRun = now();

    char timeDisplay[9] = "00:00:00";
    char dateDisplay[15] = "DDD 00/00/0000";

    if (M->isInMenu())
    {
        lastMenuRun = now();
        SafeBLon(LCD_BL);
    }

    if (!M->isInMenu() && (now() > lastMenuRun + LCD_BL_TIMEOUT))
    {
        SafeBLoff(LCD_BL);
    }

    if (!M->isInMenu() && (now() != prevDisplayTime))
    {
        prevDisplayTime = now();
        sprintf(timeDisplay, "%02u:%02u:%02u", hour(), minute(), second());
        lcd.setCursor(0, 0);
        lcd.print(timeDisplay);

        sprintf(dateDisplay, "%s %02u/%02u/%u", weekdays[weekday() - 1], day(), month(), year());
        lcd.setCursor(0, 1);
        lcd.print(dateDisplay);
        DPRINT(DEBUG_FF_UI, DEBUG_HIGH, dateDisplay);
        DPRINT(DEBUG_FF_UI, DEBUG_HIGH, " ");
        DPRINTLN(DEBUG_FF_UI, DEBUG_HIGH, timeDisplay);
    }

    M->runMenu(); // just run the menu code each loop

    for (int i = 0; i < NUM_SWITCHES; i++){
        if (digitalRead(m_switches_pins[i]) != m_switches_last_state[i]){
            m_switches_last_state[i] = digitalRead(m_switches_pins[i]);
            if (m_switches_callback != NULL){
                (*m_switches_callback)(i, !m_switches_last_state[i]);
            }
        }
    }

}

void UserInterface::setCallback(void (*callback_ptr)(int, bool)){
        m_switches_callback = callback_ptr;
}

void UserInterface::removeCallback(unsigned int switch_id){
        m_switches_callback = NULL;
}