#include <Controller.h>
//#include <UserInterface.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <unity.h>
#include <Debug.h>

#define VALVE_0 0

const uint32_t BAUD_RATE = 115200; // Serial Monitor speed setting

Controller &controller = Controller::getInstance();
//UserInterface &userInterface = UserInterface::getInstance();


time_t test_start_time;
time_t test_end_time;

struct alarmParameters{
    int hour;
    int minute;
    int durationMinutes;
    int reapeatEveryHours;
    bool isActive;
} test_valves_init[CTRL_MAX_NUM_VALVES];

void test_common (void) {

    time_t current_time;
    time_t valve_on_time;
    time_t valve_off_time;

    time_t valve_next_trigger[CTRL_MAX_NUM_VALVES];

    char message[50];

    setTime(test_start_time);

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        controller.disable(i);
        controller.setHour(i, test_valves_init[i].hour);
        controller.setMinute(i, test_valves_init[i].minute);
        controller.setDuration(i, test_valves_init[i].durationMinutes);
        controller.setRepeat(i, test_valves_init[i].reapeatEveryHours);
        if (test_valves_init[i].isActive) {
            controller.enable(i);
            TEST_ASSERT_TRUE(controller.isActive(i));
        } else {
            TEST_ASSERT_FALSE(controller.isActive(i));
        }

        TEST_ASSERT_FALSE(controller.isValveOn(i));

        valve_next_trigger[i] = previousMidnight(now()) + controller.getHour(i) * SECS_PER_HOUR + controller.getMinute(i) * SECS_PER_MIN;
        if (valve_next_trigger[i] < now()){
            valve_next_trigger[i] = nextMidnight(now()) + controller.getHour(i) * SECS_PER_HOUR + controller.getMinute(i) * SECS_PER_MIN;
        }
    }

    while ((current_time = now()) < test_end_time)
    {
        //userInterface.loop();
        Alarm.delay(0);

        for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
        {
            valve_on_time = valve_next_trigger[i];
            valve_off_time = valve_on_time + controller.getDuration(i) * SECS_PER_MIN;
            // Serial.print("Valve on:");
            // Serial.print(hour(valve_on_time));
            // Serial.print(":");
            // Serial.println(minute(valve_on_time));
            // Serial.print("Valve off:");
            // Serial.print(hour(valve_off_time));
            // Serial.print(":");
            // Serial.println(minute(valve_off_time));

            if (current_time > valve_off_time) {
                valve_next_trigger[i] += controller.getRepeat(i) * SECS_PER_HOUR;
                valve_on_time = valve_next_trigger[i];
                valve_off_time = valve_on_time + controller.getDuration(i) * SECS_PER_MIN;
                // Serial.print("Valve on:");
                // Serial.print(day(valve_on_time));
                // Serial.print("/");
                // Serial.print(month(valve_on_time));
                // Serial.print("/");
                // Serial.print(year(valve_on_time));
                // Serial.print(" ");
                // Serial.print(hour(valve_on_time));
                // Serial.print(":");
                // Serial.println(minute(valve_on_time));
                // Serial.print("Valve off:");
                // Serial.print(day(valve_on_time));
                // Serial.print("/");
                // Serial.print(month(valve_on_time));
                // Serial.print("/");
                // Serial.print(year(valve_on_time));
                // Serial.print(" ");
                // Serial.print(hour(valve_off_time));
                // Serial.print(":");
                // Serial.println(minute(valve_off_time));
            }

            if ((current_time >= valve_on_time) && (current_time < valve_off_time) && test_valves_init[i].isActive)
            {
                sprintf(message, "%02u/%02u/%u - %02u:%02u - ERROR: valve %u is not on!", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time), i);
                TEST_ASSERT_TRUE_MESSAGE(controller.isValveOn(i), message);
            }
            else
            {
                sprintf(message, "%02u/%02u/%u - %02u:%02u - ERROR: valve %u is not off!", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time), i);
                TEST_ASSERT_FALSE_MESSAGE(controller.isValveOn(i), message);
            }
        }

        setTime(now() + 60);
    }
}

void test_controller_different_periods(void)
{
    tmElements_t t;
    t.Day = 12;
    t.Month = 06;
    t.Year = 50;
    t.Hour = 20;
    t.Minute = 0;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 3ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {9, 30, 90, 12, true};
    test_valves_init[1] = {10, 30, 60, 24, true};
    test_valves_init[2] = {14, 45, 90, 48, true};

    test_common();
}

void test_controller_all_same(void)
{
    tmElements_t t;
    t.Day = 31;
    t.Month = 12;
    t.Year = 50;
    t.Hour = 20;
    t.Minute = 15;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 3ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {9, 30, 60, 24, true};
    test_valves_init[1] = {9, 30, 60, 24, true};
    test_valves_init[2] = {9, 30, 60, 24, true};

    test_common();
}

void test_controller_midnight(void)
{
    tmElements_t t;
    t.Day = 31;
    t.Month = 12;
    t.Year = 50;
    t.Hour = 20;
    t.Minute = 15;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 3ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {23, 59, 60, 24, true};
    test_valves_init[1] = {11, 50, 15, 12, true};
    test_valves_init[2] = {23, 30, 30, 24, true};

    test_common();
}

void test_controller_normal_case(void)
{
    tmElements_t t;
    t.Day = 15;
    t.Month = 06;
    t.Year = 50;
    t.Hour = 17;
    t.Minute = 12;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 60ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {17, 0, 15, 24, true};
    test_valves_init[1] = {17, 15, 15, 24, true};
    test_valves_init[2] = {17, 30, 30, 120, true};

    test_common();
}

void test_controller_valve_disabled_case(void)
{
    tmElements_t t;
    t.Day = 15;
    t.Month = 06;
    t.Year = 50;
    t.Hour = 17;
    t.Minute = 12;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 2ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {17, 0, 15, 24, true};
    test_valves_init[1] = {17, 15, 15, 24, false};
    test_valves_init[2] = {17, 30, 30, 120, true};

    test_common();
}

void test_controller_save_load()
{
    test_valves_init[0] = {12, 0, 15, 24, true};
    test_valves_init[1] = {14, 15, 15, 24, true};
    test_valves_init[2] = {20, 30, 30, 120, true};
    
    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        controller.setHour(i, test_valves_init[i].hour);
        controller.setMinute(i, test_valves_init[i].minute);
        controller.setDuration(i, test_valves_init[i].durationMinutes);
        controller.setRepeat(i, test_valves_init[i].reapeatEveryHours);
        controller.enable(i);
    }

    controller.save();
    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        controller.setHour(i, 0);
        controller.setMinute(i, 0);
        controller.setDuration(i, 0);
        controller.setRepeat(i, 0);
        controller.disable(i);
    }
    controller.load();

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        TEST_ASSERT_EQUAL_INT(test_valves_init[i].hour, controller.getHour(i));
        TEST_ASSERT_EQUAL_INT(test_valves_init[i].minute, controller.getMinute(i));
        TEST_ASSERT_EQUAL_INT(test_valves_init[i].durationMinutes, controller.getDuration(i));
        TEST_ASSERT_EQUAL_INT(test_valves_init[i].reapeatEveryHours, controller.getRepeat(i));
        TEST_ASSERT_TRUE(controller.isActive(i));
    }
}

void test_disabling_enabling_valve (void) {
    

    time_t current_time;
    time_t valve_on_time;
    time_t valve_off_time;

    time_t valve_next_trigger[CTRL_MAX_NUM_VALVES];

    char message[50];

    tmElements_t t;
    t.Day = 15;
    t.Month = 06;
    t.Year = 50;
    t.Hour = 12;
    t.Minute = 0;
    t.Second = 0;

    test_start_time = makeTime(t);
    test_end_time = test_start_time + 2ul * 24ul * 60ul * 60ul;

    test_valves_init[0] = {17, 0, 15, 24, true};
    test_valves_init[1] = {17, 15, 15, 24, true};
    test_valves_init[2] = {17, 30, 30, 120, true};

    setTime(test_start_time);

    for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
    {
        controller.disable(i);
        controller.setHour(i, test_valves_init[i].hour);
        controller.setMinute(i, test_valves_init[i].minute);
        controller.setDuration(i, test_valves_init[i].durationMinutes);
        controller.setRepeat(i, test_valves_init[i].reapeatEveryHours);
        if (test_valves_init[i].isActive) {
            controller.enable(i);
            TEST_ASSERT_TRUE(controller.isActive(i));
        } else {
            TEST_ASSERT_FALSE(controller.isActive(i));
        }

        TEST_ASSERT_FALSE(controller.isValveOn(i));

        valve_next_trigger[i] = previousMidnight(now()) + controller.getHour(i) * SECS_PER_HOUR + controller.getMinute(i) * SECS_PER_MIN;
        if (valve_next_trigger[i] < now()){
            valve_next_trigger[i] = nextMidnight(now()) + controller.getHour(i) * SECS_PER_HOUR + controller.getMinute(i) * SECS_PER_MIN;
        }
        Serial.print("Next Trigger for valve ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(year(valve_next_trigger[i]));
        Serial.print(month(valve_next_trigger[i]));
        Serial.print(day(valve_next_trigger[i]));
        Serial.print(" ");
        Serial.print(hour(valve_next_trigger[i]));
        Serial.print(":");
        Serial.print(minute(valve_next_trigger[i]));
        Serial.print(":");
        Serial.println(second(valve_next_trigger[i]));
    }

    while ((current_time = now()) < test_end_time)
    {
        Alarm.delay(0);

        for (int i = 0; i < CTRL_MAX_NUM_VALVES; i++)
        {

            valve_on_time = valve_next_trigger[i];
            valve_off_time = valve_on_time + controller.getDuration(i) * SECS_PER_MIN;

            if (current_time > valve_off_time) {
                valve_next_trigger[i] += controller.getRepeat(i) * SECS_PER_HOUR;
                Serial.print("Next Trigger for valve ");
                Serial.print(i);
                Serial.print(": ");
                Serial.print(year(valve_next_trigger[i]));
                Serial.print(month(valve_next_trigger[i]));
                Serial.print(day(valve_next_trigger[i]));
                Serial.print(" ");
                Serial.print(hour(valve_next_trigger[i]));
                Serial.print(":");
                Serial.print(minute(valve_next_trigger[i]));
                Serial.print(":");
                Serial.println(second(valve_next_trigger[i]));
                valve_on_time = valve_next_trigger[i];
                valve_off_time = valve_on_time + controller.getDuration(i) * SECS_PER_MIN;

                if (!test_valves_init[i].isActive){
                    test_valves_init[i].isActive = true;
                    controller.enable(i);
                }

            }

            if ((current_time >= valve_on_time) && (current_time < valve_off_time) && test_valves_init[i].isActive)
            {
                sprintf(message, "%02u/%02u/%u - %02u:%02u - ERROR: valve %u is not on!", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time), i);
                TEST_ASSERT_TRUE_MESSAGE(controller.isValveOn(i), message);
                test_valves_init[i].isActive = false;
                controller.disable(i);
            }
            else
            {
                if (controller.isValveOn(i)) {
                    Serial.print(day(current_time));
                    Serial.print(hour(current_time));
                    Serial.print(":");
                    Serial.print(minute(current_time));
                    Serial.print(":");
                    Serial.println(second(current_time));

                    
                    Serial.print(year(valve_on_time));
                    Serial.print(month(valve_on_time));
                    Serial.print(day(valve_on_time));
                    Serial.print(" ");
                    Serial.print(hour(valve_on_time));
                    Serial.print(":");
                    Serial.print(minute(valve_on_time));
                    Serial.print(":");
                    Serial.println(second(valve_on_time));

                    Serial.print(hour(valve_off_time));
                    Serial.print(":");
                    Serial.print(minute(valve_off_time));
                    Serial.print(":");
                    Serial.println(second(valve_off_time));

                    Serial.println(test_valves_init[i].isActive? "true": "false");
                }
                sprintf(message, "%02u/%02u/%u - %02u:%02u - ERROR: valve %u is not off!", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time), i);
                TEST_ASSERT_FALSE_MESSAGE(controller.isValveOn(i), message);
            }
        }

        setTime(now() + 60);
    }
}

void setup(){
    
    Serial.begin(BAUD_RATE);
    Serial.println("Start testing ...");

    setTime(12,0,0,1,1,2020);
    controller.setup();
    //userInterface.begin();
    
    UNITY_BEGIN();

    RUN_TEST(test_controller_save_load);
    RUN_TEST(test_controller_different_periods);
    RUN_TEST(test_controller_all_same);
    RUN_TEST(test_controller_midnight);
    RUN_TEST(test_controller_normal_case);
    RUN_TEST(test_controller_valve_disabled_case);
    //RUN_TEST(test_disabling_enabling_valve);
}

void loop(){
    
    UNITY_END();
}