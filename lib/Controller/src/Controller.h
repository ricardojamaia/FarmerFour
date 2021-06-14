//
// Created by Ricardo Maia  on 09/06/2020.
//

#ifndef FARMFOUR_CONTROLLER_H
#define FARMFOUR_CONTROLLER_H

#define CTRL_MAX_NUM_VALVES 3
#define CTRL_VALVE_OUT_PINS {24, 26, 28}

#define CTRL_PUMP_OUT_PIN   22

#define FF_CTRL_EEPROM_ADDR_CRC 0x00
#define FF_CTRL__EEPROM_ADDR_CONFIG 0x10

#include <TimeLib.h>
#include <TimeAlarms.h>

// TODO: mValve on and isValveOn does not seem to be used other than for testing. Read pin value to check if the valve is on.

class Controller {
public:
    static Controller &getInstance() {
        static Controller instance;
        return instance;
    }

    void setHour(int valveId, int hour) { mValveConfigArray[valveId].hour = hour; }
    void setMinute(int valveId, int minute){ mValveConfigArray[valveId].minute = minute; }
    void setRepeat(int valveId, int repeatEveryHours){ mValveConfigArray[valveId].repeatEveryHours = repeatEveryHours; }
    void setDuration(int valveId, int durationMinutes){ mValveConfigArray[valveId].durationMinutes = durationMinutes; }
    void enable(int valveId);
    void disable(int valveId);

    void turnOnValve(int valveId);
    void turnOffValve(int valveId);

    int getHour(int valveId){ return mValveConfigArray[valveId].hour; }
    int getMinute(int valveId){ return mValveConfigArray[valveId].minute; }
    int getRepeat(int valveId){ return mValveConfigArray[valveId].repeatEveryHours; }
    int getDuration(int valveId){ return mValveConfigArray[valveId].durationMinutes; }
    bool isActive(int valveId){ return mValveConfigArray[valveId].active; }
    bool isValveOn(int valveId){ return mValveOn[valveId]; }

    void setup();

    void save();
    void load();

    static void _alarmHandler(){
        Controller::getInstance().alarmHandler();
    }

private:

    typedef struct {
        bool active;
        int hour;
        int minute;
        int repeatEveryHours;
        int durationMinutes;
    } ValveConfigT;

    Controller(){};
    Controller(Controller const &);    // Don't Implement.
    void operator=(Controller const &);       // Don't implement

    ValveConfigT mValveConfigArray[CTRL_MAX_NUM_VALVES];
    AlarmId onAlarmId[CTRL_MAX_NUM_VALVES];
    AlarmId offAlarmId[CTRL_MAX_NUM_VALVES];
    int mValvePins[CTRL_MAX_NUM_VALVES] = CTRL_VALVE_OUT_PINS;
    bool mValveOn[CTRL_MAX_NUM_VALVES];

    void alarmHandler();
};

#endif //FARMFOUR_CONTROLLER_H
