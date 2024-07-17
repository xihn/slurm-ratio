#include "print_debug_test.h"
#include "unity/unity.c"

void setUp(void) {
  read_config_file(config_file);
}

void tearDown(void) {
    // clean stuff up here
}


// CPU COUNT TESTS
void test_maxCpuCount(void) {
    TEST_ASSERT_TRUE(_check_ratio("es1", "gpu:V100:2", 16));
    TEST_ASSERT_FALSE(_check_ratio("es1", "gpu:V100:2", 17));
}

void test_minCpuCount(void) {
    TEST_ASSERT_FALSE(_check_ratio("es1", "gpu:V100:2", 4));
}

void test_OutOfBoundsCpu(void) {

}

// RATIO TESTS
 
void test_ratio(void) {

}

void test_ratioOutOfBounds(void) {

}

// GRES & GPU TESTS
void test_noSpecifiedCard(void) {

}

void test_largeGpuCount(void) {

}

void test_invalidGRES(void) {
    // gpu doesnt exist in config
}

// CONFIG TESTS

void test_configMissingGpu(void) {

}

void test_normalConfigRead(void) {

}

void test_noConfigFile(void) {

}


// not needed when using
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_maxCpuCount);
    RUN_TEST(test_minCpuCount);
    return UNITY_END();
}