/**
 * @file task_queue_producer_test.cpp
 * @brief Unit tests for SafeQueue and TaskQueueProducer cross-task
 *        communication. These tests exercise the scenarios that crash
 *        on single-core ESP32-C3.
 */

#include <Arduino.h>

#include "ReactESP.h"
#include "sensesp/system/task_queue_producer.h"
#include "unity.h"

using namespace sensesp;

// ---------------------------------------------------------------------------
// SafeQueue basic operations
// ---------------------------------------------------------------------------

void test_safe_queue_starts_empty() {
  SafeQueue<int> q;
  TEST_ASSERT_TRUE(q.empty());
  TEST_ASSERT_EQUAL(0, q.size());
}

void test_safe_queue_push_pop() {
  SafeQueue<int> q;
  q.push(42);
  TEST_ASSERT_FALSE(q.empty());
  TEST_ASSERT_EQUAL(1, q.size());

  int value = 0;
  bool got = q.pop(value, 0);
  TEST_ASSERT_TRUE(got);
  TEST_ASSERT_EQUAL(42, value);
  TEST_ASSERT_TRUE(q.empty());
}

void test_safe_queue_fifo_order() {
  SafeQueue<int> q;
  for (int i = 0; i < 10; i++) {
    q.push(i);
  }
  TEST_ASSERT_EQUAL(10, q.size());

  for (int i = 0; i < 10; i++) {
    int value = -1;
    bool got = q.pop(value, 0);
    TEST_ASSERT_TRUE(got);
    TEST_ASSERT_EQUAL(i, value);
  }
  TEST_ASSERT_TRUE(q.empty());
}

void test_safe_queue_pop_from_empty_returns_false() {
  SafeQueue<int> q;
  int value = 99;
  bool got = q.pop(value, 0);
  TEST_ASSERT_FALSE(got);
  TEST_ASSERT_EQUAL(99, value);  // value unchanged
}

// ---------------------------------------------------------------------------
// SafeQueue concurrent access from two FreeRTOS tasks
// ---------------------------------------------------------------------------

static const int kConcurrentCount = 500;

struct ConcurrentTestContext {
  SafeQueue<int>* queue;
  SemaphoreHandle_t done;
  int produced_count;
  int consumed_values[kConcurrentCount];
  int consumed_count;
};

static void producer_task(void* param) {
  auto* ctx = static_cast<ConcurrentTestContext*>(param);
  for (int i = 0; i < kConcurrentCount; i++) {
    ctx->queue->push(i);
    // Yield occasionally to interleave with consumer
    if (i % 10 == 0) {
      vTaskDelay(1);
    }
  }
  ctx->produced_count = kConcurrentCount;
  xSemaphoreGive(ctx->done);
  vTaskDelete(nullptr);
}

void test_safe_queue_concurrent_access() {
  SafeQueue<int> queue(kConcurrentCount + 1);
  ConcurrentTestContext ctx = {};
  ctx.queue = &queue;
  ctx.done = xSemaphoreCreateBinary();
  ctx.produced_count = 0;
  ctx.consumed_count = 0;

  // Start producer on a separate task
  TaskHandle_t task_handle;
  xTaskCreate(producer_task, "producer", 4096, &ctx, 1, &task_handle);

  // Consumer: poll until all values received (with timeout)
  unsigned long start = millis();
  while (ctx.consumed_count < kConcurrentCount && millis() - start < 5000) {
    int value;
    while (queue.pop(value, 0)) {
      if (ctx.consumed_count < kConcurrentCount) {
        ctx.consumed_values[ctx.consumed_count++] = value;
      }
    }
    vTaskDelay(1);
  }

  // Wait for producer to finish
  xSemaphoreTake(ctx.done, pdMS_TO_TICKS(2000));

  TEST_ASSERT_EQUAL(kConcurrentCount, ctx.produced_count);
  TEST_ASSERT_EQUAL(kConcurrentCount, ctx.consumed_count);

  // Verify FIFO order preserved
  for (int i = 0; i < kConcurrentCount; i++) {
    TEST_ASSERT_EQUAL(i, ctx.consumed_values[i]);
  }

  vSemaphoreDelete(ctx.done);
}

// ---------------------------------------------------------------------------
// SafeQueue stress test: rapid interleaved access from two tasks
// ---------------------------------------------------------------------------

static const int kStressCount = 1000;

struct StressTestContext {
  SafeQueue<int>* queue;
  SemaphoreHandle_t done;
  int push_count;
};

static void stress_producer_task(void* param) {
  auto* ctx = static_cast<StressTestContext*>(param);
  for (int i = 0; i < kStressCount; i++) {
    ctx->queue->push(i);
    // No delay — maximum contention
  }
  ctx->push_count = kStressCount;
  xSemaphoreGive(ctx->done);
  vTaskDelete(nullptr);
}

void test_safe_queue_stress() {
  SafeQueue<int> queue(kStressCount + 1);
  StressTestContext ctx = {};
  ctx.queue = &queue;
  ctx.done = xSemaphoreCreateBinary();
  ctx.push_count = 0;

  TaskHandle_t task_handle;
  xTaskCreate(stress_producer_task, "stress", 4096, &ctx, 1, &task_handle);

  int pop_count = 0;
  int last_value = -1;
  unsigned long start = millis();

  while (pop_count < kStressCount && millis() - start < 10000) {
    int value;
    while (queue.pop(value, 0)) {
      // Values must arrive in order
      TEST_ASSERT_GREATER_THAN(last_value, value);
      last_value = value;
      pop_count++;
    }
    // Busy-poll with minimal delay
    taskYIELD();
  }

  xSemaphoreTake(ctx.done, pdMS_TO_TICKS(2000));

  TEST_ASSERT_EQUAL(kStressCount, ctx.push_count);
  TEST_ASSERT_EQUAL(kStressCount, pop_count);

  vSemaphoreDelete(ctx.done);
}

// ---------------------------------------------------------------------------
// TaskQueueProducer: set() from producer task, emit on consumer event loop
// ---------------------------------------------------------------------------

static const int kTQPCount = 50;

struct TQPTestContext {
  TaskQueueProducer<int>* tqp;
  SemaphoreHandle_t done;
};

static void tqp_producer_task(void* param) {
  auto* ctx = static_cast<TQPTestContext*>(param);
  for (int i = 0; i < kTQPCount; i++) {
    ctx->tqp->set(i);
    vTaskDelay(1);
  }
  xSemaphoreGive(ctx->done);
  vTaskDelete(nullptr);
}

void test_task_queue_producer_cross_task_emit() {
  auto event_loop = std::make_shared<reactesp::EventLoop>();

  // Create TQP with poll rate 0 (every tick) on our event loop
  auto tqp = std::make_shared<TaskQueueProducer<int>>(0, event_loop, 0);

  // Track emitted values
  int received_count = 0;
  int received_values[kTQPCount];
  memset(received_values, -1, sizeof(received_values));

  tqp->attach([&received_count, &received_values, &tqp]() {
    if (received_count < kTQPCount) {
      received_values[received_count++] = tqp->get();
    }
  });

  TQPTestContext ctx = {};
  ctx.tqp = tqp.get();
  ctx.done = xSemaphoreCreateBinary();

  TaskHandle_t task_handle;
  xTaskCreate(tqp_producer_task, "tqp_prod", 4096, &ctx, 1, &task_handle);

  // Run the consumer event loop until all values received or timeout
  unsigned long start = millis();
  while (received_count < kTQPCount && millis() - start < 5000) {
    event_loop->tick();
    vTaskDelay(1);
  }

  xSemaphoreTake(ctx.done, pdMS_TO_TICKS(2000));

  TEST_ASSERT_EQUAL(kTQPCount, received_count);

  // Verify values arrived in order
  for (int i = 0; i < kTQPCount; i++) {
    TEST_ASSERT_EQUAL(i, received_values[i]);
  }

  vSemaphoreDelete(ctx.done);
}

// ---------------------------------------------------------------------------
// TaskQueueProducer: multiple rapid sets without yielding
// ---------------------------------------------------------------------------

static void tqp_burst_producer_task(void* param) {
  auto* ctx = static_cast<TQPTestContext*>(param);
  // Burst all values without yielding — stresses the queue
  for (int i = 0; i < kTQPCount; i++) {
    ctx->tqp->set(i);
  }
  xSemaphoreGive(ctx->done);
  vTaskDelete(nullptr);
}

void test_task_queue_producer_burst() {
  auto event_loop = std::make_shared<reactesp::EventLoop>();
  auto tqp = std::make_shared<TaskQueueProducer<int>>(0, event_loop, 0);

  int received_count = 0;
  int received_values[kTQPCount];
  memset(received_values, -1, sizeof(received_values));

  tqp->attach([&received_count, &received_values, &tqp]() {
    if (received_count < kTQPCount) {
      received_values[received_count++] = tqp->get();
    }
  });

  TQPTestContext ctx = {};
  ctx.tqp = tqp.get();
  ctx.done = xSemaphoreCreateBinary();

  TaskHandle_t task_handle;
  xTaskCreate(tqp_burst_producer_task, "burst", 4096, &ctx, 1, &task_handle);

  unsigned long start = millis();
  while (received_count < kTQPCount && millis() - start < 5000) {
    event_loop->tick();
    vTaskDelay(1);
  }

  xSemaphoreTake(ctx.done, pdMS_TO_TICKS(2000));

  TEST_ASSERT_EQUAL(kTQPCount, received_count);
  for (int i = 0; i < kTQPCount; i++) {
    TEST_ASSERT_EQUAL(i, received_values[i]);
  }

  vSemaphoreDelete(ctx.done);
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

void setup() {
  delay(2000);  // Allow serial connection to stabilize

  UNITY_BEGIN();

  // SafeQueue basics
  RUN_TEST(test_safe_queue_starts_empty);
  RUN_TEST(test_safe_queue_push_pop);
  RUN_TEST(test_safe_queue_fifo_order);
  RUN_TEST(test_safe_queue_pop_from_empty_returns_false);

  // SafeQueue cross-task
  RUN_TEST(test_safe_queue_concurrent_access);
  RUN_TEST(test_safe_queue_stress);

  // TaskQueueProducer cross-task
  RUN_TEST(test_task_queue_producer_cross_task_emit);
  RUN_TEST(test_task_queue_producer_burst);

  UNITY_END();
}

void loop() {}
