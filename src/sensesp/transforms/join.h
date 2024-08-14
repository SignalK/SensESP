#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_JOIN_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_JOIN_H_

#include <elapsedMillis.h>
#include <tuple>

#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/valueproducer.h"
#include "transform.h"

namespace sensesp {

////////////////////////////
// Base class for Join. This is needed until Arduino ESP32 Core supports C++14
// and template pack indices: https://stackoverflow.com/a/55807564/2999754

template <int N>
class JoinBase {
 public:
  JoinBase(long max_age = 0) : max_age_{max_age} {
    for (int i = 0; i < N; i++) {
      age_[i] = max_age;
    }
  }

 protected:
  elapsedMillis age_[N];
  long max_age_;

  virtual void emit_tuple() = 0;

  void check_emit() {
    for (int i = 0; i < N; i++) {
      if (max_age_ != 0 && age_[i] > max_age_) {
        return;
      }
    }
    emit_tuple();
  }
};

/**
 * @brief Join two producer values into a tuple.
 *
 * Joins the connected producers' values into a tuple. The tuple is emitted
 * when any producer emits a new value and none of the values have aged more
 * than max_age milliseconds.
 *
 */
template <typename T1, typename T2>
class Join : public JoinBase<2>, public ValueProducer<std::tuple<T1, T2>> {
 public:
  Join(long max_age = 0) : JoinBase<2>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>> consumers = {
      LambdaConsumer<T1>([this](T1 value) {
        std::get<0>(values) = value;
        age_[0] = 0;
        check_emit();
      }),
      LambdaConsumer<T2>([this](T2 value) {
        std::get<1>(values) = value;
        age_[1] = 0;
        check_emit();
      })};

 protected:
  std::tuple<T1, T2> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Merge three producer values into a tuple.
 *
 * Merges the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3>
class Join3 : public JoinBase<3>, public ValueProducer<std::tuple<T1, T2, T3>> {
 public:
  Join3(long max_age = 0) : JoinBase<3>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Merge four producer values into a tuple.
 *
 * Merges the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3, typename T4>
class Join4 : public JoinBase<4>,
              public ValueProducer<std::tuple<T1, T2, T3, T4>> {
 public:
  Join4(long max_age = 0) : JoinBase<4>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>,
             LambdaConsumer<T4>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T4>([this](T4 value) {
                     std::get<3>(values) = value;
                     age_[3] = 0;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3, T4> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Merge five producer values into a tuple.
 *
 * Merges the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class Join5 : public JoinBase<5>,
              public ValueProducer<std::tuple<T1, T2, T3, T4, T5>> {
 public:
  Join5(long max_age = 0) : JoinBase<5>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>,
             LambdaConsumer<T4>, LambdaConsumer<T5>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T4>([this](T4 value) {
                     std::get<3>(values) = value;
                     age_[3] = 0;
                     check_emit();
                   }),
                   LambdaConsumer<T5>([this](T5 value) {
                     std::get<4>(values) = value;
                     age_[4] = 0;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3, T4, T5> values;

  void emit_tuple() override { this->emit(values); }
};

}  // namespace sensesp

#endif
