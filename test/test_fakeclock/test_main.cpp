#include <Arduino.h>
#include <unity.h>
#include <TestUtil.h>

#include <FakeClock.h>
#include <DriveController.h>

void test_advance()
{
    FakeClock clock;

    clock.advance(500);

    TEST_ASSERT_EQUAL(500, clock.millis());
}

void setup()
{
    TestUtil::waitForTestSerial();

    UNITY_BEGIN();
    RUN_TEST(test_advance);
    UNITY_END();
}

void loop()
{
}