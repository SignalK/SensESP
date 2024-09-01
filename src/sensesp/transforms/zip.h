#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_ZIP_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_ZIP_H_

#include <elapsedMillis.h>
#include <tuple>

#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/valueproducer.h"
#include "transform.h"

namespace sensesp {

////////////////////////////
// Base class for Zip. This is needed until Arduino ESP32 Core supports C++14
// and template pack indices: https://stackoverflow.com/a/55807564/2999754

template <int N>
class ZipBase {
 public:
  ZipBase(long max_age = 0) : max_age_{max_age} {
    for (int i = 0; i < N; i++) {
      age_[i] = max_age;
      fresh_[i] = false;
    }
  }

 protected:
  elapsedMillis age_[N];
  bool fresh_[N];
  long max_age_;

  virtual void emit_tuple() = 0;

  virtual void check_emit() {
    for (int i = 0; i < N; i++) {
      if ((max_age_ != 0 && age_[i] > max_age_) || !fresh_[i]) {
        return;
      }
    }
    emit_tuple();

    for (int i = 0; i < N; i++) {
      fresh_[i] = false;
    }
  }
};

/**
 * @brief Zip two producer values into a tuple.
 *
 * Zips the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 * All producers must emit a new value before the next tuple is emitted.
 *
 */
template <typename T1, typename T2>
class Zip : public ZipBase<2>, public ValueProducer<std::tuple<T1, T2>> {
 public:
  Zip(long max_age = 0) : ZipBase<2>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>> consumers = {
      LambdaConsumer<T1>([this](T1 value) {
        std::get<0>(values) = value;
        age_[0] = 0;
        fresh_[0] = true;
        check_emit();
      }),
      LambdaConsumer<T2>([this](T2 value) {
        std::get<1>(values) = value;
        age_[1] = 0;
        fresh_[1] = true;
        check_emit();
      })};

 protected:
  std::tuple<T1, T2> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Zip three producer values into a tuple.
 *
 * Zips the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3>
class Zip3 : public ZipBase<3>, public ValueProducer<std::tuple<T1, T2, T3>> {
 public:
  Zip3(long max_age = 0) : ZipBase<3>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     fresh_[0] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     fresh_[1] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     fresh_[2] = true;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Zip four producer values into a tuple.
 *
 * Zips the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3, typename T4>
class Zip4 : public ZipBase<4>,
             public ValueProducer<std::tuple<T1, T2, T3, T4>> {
 public:
  Zip4(long max_age = 0) : ZipBase<4>(max_age) {}

  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>,
             LambdaConsumer<T4>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     fresh_[0] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     fresh_[1] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     fresh_[2] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T4>([this](T4 value) {
                     std::get<3>(values) = value;
                     age_[3] = 0;
                     fresh_[3] = true;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3, T4> values;

  void emit_tuple() override { this->emit(values); }
};

/**
 * @brief Zip five producer values into a tuple.
 *
 * Zips the connected producers' values into a tuple. The tuple is emitted
 * once all producers have emitted a new value within max_age milliseconds.
 *
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class Zip5 : public ZipBase<5>,
             public ValueProducer<std::tuple<T1, T2, T3, T4, T5>> {
 public:
  Zip5(long max_age = 0) : ZipBase<5>(max_age) {}

  // The consumers member is a tuple of LambdaConsumers. Each LambdaConsumer
  // has a different type, hence necessitating the use of a tuple.
  std::tuple<LambdaConsumer<T1>, LambdaConsumer<T2>, LambdaConsumer<T3>,
             LambdaConsumer<T4>, LambdaConsumer<T5>>
      consumers = {LambdaConsumer<T1>([this](T1 value) {
                     std::get<0>(values) = value;
                     age_[0] = 0;
                     fresh_[0] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T2>([this](T2 value) {
                     std::get<1>(values) = value;
                     age_[1] = 0;
                     fresh_[1] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T3>([this](T3 value) {
                     std::get<2>(values) = value;
                     age_[2] = 0;
                     fresh_[2] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T4>([this](T4 value) {
                     std::get<3>(values) = value;
                     age_[3] = 0;
                     fresh_[3] = true;
                     check_emit();
                   }),
                   LambdaConsumer<T5>([this](T5 value) {
                     std::get<4>(values) = value;
                     age_[4] = 0;
                     fresh_[4] = true;
                     check_emit();
                   })};

 protected:
  std::tuple<T1, T2, T3, T4, T5> values;

  void emit_tuple() override { this->emit(values); }
};

}  // namespace sensesp

#endif
