//
// Created by Ricardo Maia  on 09/06/2020.
//

#include <Controller.h>
#include <Debug.h>
#include <TimeLib.h>
#include <EEPROM.h>

void Controller::enable(int valveId)
{
    time_t timeToAlarm;
    time_t currentTime = now();

    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, F("Enabling valve "));
    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, valveId);
    DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, F("."));

    /* If there is still an alarm free it */
    if (onAlarmId[valveId] != dtINVALID_ALARM_ID)
    {
        /* Disable and free the alarm */
        Alarm.free(onAlarmId[valveId]);
        onAlarmId[valveId] = dtINVALID_ALARM_ID;
    }

    if (offAlarmId[valveId] != dtINVALID_ALARM_ID)
    {
        /* Disable and free the alarm */
        Alarm.free(offAlarmId[valveId]);
        offAlarmId[valveId] = dtINVALID_ALARM_ID;
    }

    DPRINT(DEBUG_FF_CTRL, DEBUG_MEDIUM, F("Creating an alarm for valve "));
    DPRINT(DEBUG_FF_CTRL, DEBUG_MEDIUM, valveId);
    DPRINTLN(DEBUG_FF_CTRL, DEBUG_MEDIUM, F("."));

    timeToAlarm = previousMidnight(currentTime) +
                  mValveConfigArray[valveId].hour * SECS_PER_HOUR +
                  mValveConfigArray[valveId].minute * SECS_PER_MIN;
    if (timeToAlarm > currentTime)
    {
        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F("Alarm will start in "));
        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, timeToAlarm - currentTime);
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F(" seconds."));

        onAlarmId[valveId] = Alarm.timerOnce(timeToAlarm - currentTime,
                                             Controller::_alarmHandler);
    }
    else
    {
        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F("Alarm will start tomorrow at "));
        DPRINT_HH_MM(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[valveId].hour, mValveConfigArray[valveId].minute);
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("."));

        onAlarmId[valveId] = Alarm.alarmOnce(
            mValveConfigArray[valveId].hour,
            mValveConfigArray[valveId].minute,
            0,
            Controller::_alarmHandler);
    }
    if (onAlarmId[valveId] == dtINVALID_ALARM_ID)
    {
        DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, F("Unable to create an On Alarm for valve "));
        DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, valveId);
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, F("."));
    } else {
        mValveConfigArray[valveId].active = true;
    }
}

void Controller::disable(int valveId)
{
    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, F("Disabling valve "));
    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, valveId);
    DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, F("."));

    if (!mValveConfigArray[valveId].active){
        return;
    }

    if (onAlarmId[valveId] != dtINVALID_ALARM_ID)
    {
        /* Disable and free the alarm */
        Alarm.free(onAlarmId[valveId]);
        onAlarmId[valveId] = dtINVALID_ALARM_ID;
    }

    // TODO: Do a test case for this scenario
    if (offAlarmId[valveId] != dtINVALID_ALARM_ID)
    {
        /* Disable and free the alarm */
        Alarm.free(offAlarmId[valveId]);
        offAlarmId[valveId] = dtINVALID_ALARM_ID;
    }

    digitalWrite(mValvePins[valveId], HIGH);
    mValveOn[valveId] = false;
    mValveConfigArray[valveId].active = false;

    int allValvesOff = true;
    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        /* Check if there is a valve on. Active LOW */
        if (digitalRead(mValvePins[i]) == LOW)
        {
            allValvesOff = false;
        }
    }
    if (allValvesOff)
    {
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("All valves off. Turning Pump off."));
        digitalWrite(CTRL_PUMP_OUT_PIN, HIGH);
    }
}

void Controller::setup()
{
    // When pin is set to OUTPUT it will be put at LOW by default and activate the valve. Set first to INPUT_PULLUP
    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        mValveConfigArray[i].active = false;

        pinMode(mValvePins[i], INPUT_PULLUP);
        pinMode(mValvePins[i], OUTPUT);

        mValveOn[i] = false;

        onAlarmId[i] = dtINVALID_ALARM_ID;
        offAlarmId[i] = dtINVALID_ALARM_ID;
        mValveConfigArray[i].hour = 18;
        mValveConfigArray[i].minute = 00;
        mValveConfigArray[i].repeatEveryHours = 24;
        mValveConfigArray[i].durationMinutes = 15;
    }
    // Set pin mode for pump. Set first to INPUT_PULLUP to ensure output is set to HIGH when we set pin to OUTPUT
    pinMode(CTRL_PUMP_OUT_PIN, INPUT_PULLUP);
    pinMode(CTRL_PUMP_OUT_PIN, OUTPUT);
}

void Controller::alarmHandler()
{
    bool allValvesOff = true;

    AlarmId alarmId = Alarm.getTriggeredAlarmId();

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        if (alarmId == onAlarmId[i])
        {
            /** Turn the valve on */
            DPRINT_DATE(DEBUG_FF_CTRL, DEBUG_MEDIUM, "Switching on valve ");
            DPRINT(DEBUG_FF_CTRL, DEBUG_MEDIUM, i);
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_MEDIUM, ".");

            digitalWrite(mValvePins[i], LOW);
            mValveOn[i] = true;

            /** Set an alarm for next On event */
            onAlarmId[i] = Alarm.timerOnce(
                mValveConfigArray[i].repeatEveryHours * SECS_PER_HOUR,
                Controller::_alarmHandler);

            if (onAlarmId[i] == dtINVALID_ALARM_ID)
            {
                DPRINT_DATE(DEBUG_FF_CTRL, DEBUG_LOW, "Unable to create an On Alarm for valve ");
                DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, i);
                DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, ".");
                disable(i);
            }

            /** Set the alarm to turn off the valve */
            offAlarmId[i] = Alarm.timerOnce(
                mValveConfigArray[i].durationMinutes * SECS_PER_MIN,
                Controller::_alarmHandler);

            if (offAlarmId[i] == dtINVALID_ALARM_ID)
            {
                DPRINT_DATE(DEBUG_FF_CTRL, DEBUG_LOW, "Unable to create an Off Alarm for valve ");
                DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, i);
                DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, ".");
                disable(i);
            }
        }
        else if (alarmId == offAlarmId[i])
        {
            /* Turn off the valve */
            DPRINT_DATE(DEBUG_FF_CTRL, DEBUG_MEDIUM, "Switching off valve ");
            DPRINT(DEBUG_FF_CTRL, DEBUG_MEDIUM, i);
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_MEDIUM, ".");

            digitalWrite(mValvePins[i], HIGH);
            mValveOn[i] = false;
            /* onceOnly alarms are free automatically */
            /* Alarm was freed so let's invalidate it */
            offAlarmId[i] = dtINVALID_ALARM_ID;
        }
    }

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        /* Check if there is a valve on. Active LOW */
        if (digitalRead(mValvePins[i]) == LOW)
        {
            allValvesOff = false;
        }
    }

    if (allValvesOff)
    {
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("All valves off. Turning Pump off."));
        digitalWrite(CTRL_PUMP_OUT_PIN, HIGH);
    }
}


void Controller::turnOnValve(int valveId) {

    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, F("Manually turning on valve "));
    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, valveId);
    DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, F("."));
    digitalWrite(mValvePins[valveId], LOW);
    mValveOn[valveId] = true;

    DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("Turning Pump on."));
    digitalWrite(CTRL_PUMP_OUT_PIN, LOW);
    
};

void Controller::turnOffValve(int valveId) {
    bool allValvesOff = true;

    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, F("Manually turning off valve "));
    DPRINT(DEBUG_FF_CTRL, DEBUG_LOW, valveId);
    DPRINTLN(DEBUG_FF_CTRL, DEBUG_LOW, F("."));

    digitalWrite(mValvePins[valveId], HIGH);
    mValveOn[valveId] = false;


    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        /* Check if there is a valve on. Active LOW */
        if (digitalRead(mValvePins[i]) == LOW)
        {
            allValvesOff = false;
        }
    }    

    if (allValvesOff)
    {
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("All valves off. Turning Pump off."));
        digitalWrite(CTRL_PUMP_OUT_PIN, HIGH);
    }
};


void Controller::save(){

    DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("Saving Controller state."));
#if DEBUG_MODE
    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++) {
        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F("Valve "));
        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, i);
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F(" parameters:"));

        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Active: "));
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].active ? "True" : "False");

        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Hour: "));
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].hour);

        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Minute: "));
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].minute);

        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Duration (min): "));
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].durationMinutes);

        DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Period (hours): "));
        DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].repeatEveryHours);
    }
#endif

    EEPROM.put(FF_CTRL__EEPROM_ADDR_CONFIG, mValveConfigArray);
}

void Controller::load(){

    DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F("Loading Controller state."));

    EEPROM.get(FF_CTRL__EEPROM_ADDR_CONFIG, mValveConfigArray);

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++){
        if (mValveConfigArray[i].active)
        {
            enable(i);
        }
        else
        {
            disable(i);
        }
    }

#if DEBUG_MODE
        for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
        {
            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F("Valve "));
            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, i);
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, F(" parameters:"));

            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Active: "));
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].active ? "True" : "False");

            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Hour: "));
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].hour);

            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Minute: "));
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].minute);

            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Duration (min): "));
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].durationMinutes);

            DPRINT(DEBUG_FF_CTRL, DEBUG_HIGH, F(" - Period (hours): "));
            DPRINTLN(DEBUG_FF_CTRL, DEBUG_HIGH, mValveConfigArray[i].repeatEveryHours);
        }
#endif
}