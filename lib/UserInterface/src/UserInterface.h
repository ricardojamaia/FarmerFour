//
// Created by Ricardo Maia  on 12/06/2020.
//

#ifndef FARMFOUR_USERINTERFACE_H
#define FARMFOUR_USERINTERFACE_H

#include <LiquidCrystal.h>
#include <MD_UISwitch.h>
#include <MD_Menu.h>


// LCD Display -----------------------------
#define LCD_BL_TIMEOUT 60

// LCD display definitions
#define  LCD_ROWS 2
#define  LCD_COLS 16

// LCD pin definitions
#define LCD_RS  8
#define LCD_EN  9
#define LCD_D4  4
#define LCD_D5  LCD_D4+1
#define LCD_D6  LCD_D4+2
#define LCD_D7  LCD_D4+3
#define LCD_BL  10

#define NUM_SWITCHES 3
#define SWITCHES_PINS {49, 51, 53}
#define GND_PINS {48, 50, 52}

#define SafeBLon(pin) pinMode(pin, INPUT)
#define SafeBLoff(pin) pinMode(pin, OUTPUT)

class UserInterface {
public:
    static UserInterface &getInstance() {
        static UserInterface instance;
        return instance;
    }

    void begin(MD_Menu *menu);
    void loop();
    MD_Menu::userNavAction_t navigation(uint16_t &incDelta);
    bool display(MD_Menu::userDisplayAction_t action, char *msg = nullptr);

    static MD_Menu::userNavAction_t _navigation(uint16_t &incDelta);
    static bool _display(MD_Menu::userDisplayAction_t action, char *msg = nullptr);

    void setCallback(void (*callback_ptr)(int,bool));
    void removeCallback(unsigned int switch_id);

private:
    char weekdays[7][4] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};
    MD_Menu *M;
    bool backlightOn;
    int m_switches_pins[NUM_SWITCHES];
    int m_gnd_pins[NUM_SWITCHES];
    bool m_switches_last_state[NUM_SWITCHES];

    void (*m_switches_callback) (int switch_id, bool isOn);

    UserInterface(): m_switches_pins(SWITCHES_PINS), m_gnd_pins(GND_PINS)
    {

    };
    UserInterface(UserInterface const &);    // Don't Implement.
    void operator=(UserInterface const &);       // Don't implement
};


#endif //FARMFOUR_USERINTERFACE_H
