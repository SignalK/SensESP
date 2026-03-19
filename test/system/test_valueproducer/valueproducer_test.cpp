/**
 * @file valueproducer_test.cpp
 * @brief Tests for ValueProducer::connect_to() lifecycle behavior.
 *
 * Verifies that connect_to(shared_ptr) keeps the consumer alive and that
 * values flow through the connection after the caller's shared_ptr goes
 * out of scope.
 */

#include <Arduino.h>

#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/lambda_transform.h"
#include "unity.h"

using namespace sensesp;

// ---------------------------------------------------------------------------
// Baseline: raw pointer connect_to works
// ---------------------------------------------------------------------------

void test_connect_to_raw_pointer() {
  ObservableValue<int> producer(0);
  int received = -1;

  LambdaConsumer<int> consumer([&received](int v) { received = v; });
  producer.connect_to(&consumer);

  producer.set(42);
  TEST_ASSERT_EQUAL(42, received);
}

// ---------------------------------------------------------------------------
// Baseline: reference connect_to works
// ---------------------------------------------------------------------------

void test_connect_to_reference() {
  ObservableValue<int> producer(0);
  int received = -1;

  LambdaConsumer<int> consumer([&received](int v) { received = v; });
  producer.connect_to(consumer);

  producer.set(99);
  TEST_ASSERT_EQUAL(99, received);
}

// ---------------------------------------------------------------------------
// shared_ptr consumer survives after local shared_ptr goes out of scope
// ---------------------------------------------------------------------------

void test_connect_to_shared_ptr_keeps_consumer_alive() {
  ObservableValue<int> producer(0);
  int received = -1;

  // Create shared_ptr consumer in inner scope — it goes out of scope,
  // but connect_to should keep it alive.
  {
    auto consumer = std::make_shared<LambdaConsumer<int>>(
        [&received](int v) { received = v; });
    producer.connect_to(consumer);
    // consumer shared_ptr goes out of scope here
  }

  // Emit after the local shared_ptr is gone.
  // With weak_ptr: consumer is destroyed, value is lost.
  // With strong ref: consumer is alive, value is received.
  producer.set(42);
  TEST_ASSERT_EQUAL(42, received);
}

// ---------------------------------------------------------------------------
// shared_ptr inline (make_shared in connect_to call) — the common pattern
// ---------------------------------------------------------------------------

void test_connect_to_inline_make_shared() {
  ObservableValue<int> producer(0);
  int received = -1;

  // This is the pattern used in NMEA0183IOTask and wiring functions.
  // The shared_ptr is a temporary — destroyed at end of statement.
  producer.connect_to(
      std::make_shared<LambdaConsumer<int>>(
          [&received](int v) { received = v; }));

  producer.set(77);
  TEST_ASSERT_EQUAL(77, received);
}

// ---------------------------------------------------------------------------
// shared_ptr chaining: producer → shared transform → raw consumer
// ---------------------------------------------------------------------------

void test_connect_to_shared_ptr_chain() {
  ObservableValue<int> producer(0);
  int received = -1;

  LambdaConsumer<int> final_consumer([&received](int v) { received = v; });

  // Create transform as shared_ptr, chain to raw pointer consumer.
  // The transform must stay alive for the chain to work.
  {
    auto transform = std::make_shared<LambdaTransform<int, int>>(
        [](int v) { return v * 2; });
    producer.connect_to(transform)->connect_to(&final_consumer);
    // transform shared_ptr goes out of scope here
  }

  producer.set(21);
  TEST_ASSERT_EQUAL(42, received);
}

// ---------------------------------------------------------------------------
// Multiple shared_ptr consumers on same producer
// ---------------------------------------------------------------------------

void test_connect_to_multiple_shared_ptr_consumers() {
  ObservableValue<int> producer(0);
  int received_a = -1;
  int received_b = -1;

  producer.connect_to(
      std::make_shared<LambdaConsumer<int>>(
          [&received_a](int v) { received_a = v; }));
  producer.connect_to(
      std::make_shared<LambdaConsumer<int>>(
          [&received_b](int v) { received_b = v; }));

  producer.set(55);
  TEST_ASSERT_EQUAL(55, received_a);
  TEST_ASSERT_EQUAL(55, received_b);
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_connect_to_raw_pointer);
  RUN_TEST(test_connect_to_reference);
  RUN_TEST(test_connect_to_shared_ptr_keeps_consumer_alive);
  RUN_TEST(test_connect_to_inline_make_shared);
  RUN_TEST(test_connect_to_shared_ptr_chain);
  RUN_TEST(test_connect_to_multiple_shared_ptr_consumers);

  UNITY_END();
}

void loop() {}
