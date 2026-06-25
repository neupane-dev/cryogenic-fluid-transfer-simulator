#include <stdio.h>
#include <stdbool.h>
#include "cryo_defs.h"
#include "cryo_hal.h"
#include "cryo_controller.h"

// Tracking variables for automated reporting
static int tests_run = 0;
static int tests_passed = 0;

void ASSERT_EQUAL_INT(int expected, int actual, const char* test_name) {
    tests_run++;
    if (expected == actual) {
        printf("[PASS] %s\n", test_name);
        tests_passed++;
    } else {
        printf("[FAIL] %s | Expected %d, but got %d\n", test_name, expected, actual);
    }
}

// TEST 1: Verify the software cycles correctly through nominal phases under safe conditions
void Test_Nominal_State_Transitions(void) {
    CryoSystem_t test_sys = { .current_state = STATE_IDLE };
    HAL_Test_Override_Pressure = 120.0f; // Completely safe pressure

    // Tick 1: IDLE -> PRE_CHILL
    CRYO_run_control_logic(&test_sys);
    ASSERT_EQUAL_INT(STATE_PRE_CHILL, test_sys.current_state, "Nominal Transition to PRE_CHILL");

    // Tick 2: PRE_CHILL -> PRESSURE_EQUALIZATION
    CRYO_run_control_logic(&test_sys);
    ASSERT_EQUAL_INT(STATE_PRESSURE_EQUALIZATION, test_sys.current_state, "Nominal Transition to EQUALIZATION");
}

// TEST 2: Verify instantaneous safety fault triggers if an overpressure occurs during transfer
void Test_Overpressure_Safety_Interlock(void) {
    CryoSystem_t test_sys = { .current_state = STATE_TRANSFERRING };
    
    // Force sensor beyond the 300.0 PSI threshold constraint safely handled by our HAL
    HAL_Test_Override_Pressure = 345.0f; 

    CRYO_run_control_logic(&test_sys);
    ASSERT_EQUAL_INT(STATE_EMERGENCY_FAULT, test_sys.current_state, "Overpressure Trapped and Handled Successfully");
}

int main(void) {
    printf("==================================================\n");
    printf("   RUNNING AUTOMATED FIRMWARE TEST FIXTURES      \n");
    printf("==================================================\n");

    Test_Nominal_State_Transitions();
    Test_Overpressure_Safety_Interlock();

    printf("\n==================================================\n");
    printf("TEST SUMMARY: %d/%d Tests Passed.\n", tests_passed, tests_run);
    printf("==================================================\n");

    // Return 0 if all tests pass, otherwise return failure code to environmental runners
    return (tests_passed == tests_run) ? 0 : 1;
}