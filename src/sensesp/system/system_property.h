#ifndef _SYSTEM_PROPERTY_H_
#define _SYSTEM_PROPERTY_H_
#include <ArduinoJson.h>
#include "Arduino.h"
#include "observablevalue.h"
#include "valueproducer.h"
#include "valueconsumer.h"
#include <map>

namespace sensesp {

    class SystemPropertyBase : virtual public Observable
     {
        protected:
            String name_;
        public:
            SystemPropertyBase(String name);
            String& get_name()
            {
                return name_;
            }

            virtual void set_json(const JsonObject& obj) { }
     };

     extern std::map<String, SystemPropertyBase*> systemProperties;

     template<typename T>
     class SystemProperty : public SystemPropertyBase, public ObservableValue<T>, public ValueConsumer<T>
     {
        public:
            SystemProperty(String name) : SystemPropertyBase(name)
            { }

            SystemProperty(String name, T value) : SystemPropertyBase(name)
            {
                this->ObservableValue<T>::output = value;
            }

            void set_json(const JsonObject& obj) override
            {
                obj[name_] = this->ObservableValue<T>::output;
            }

            void set_input(T new_value, uint8_t input_channel = 0) override
            {
                this->ValueProducer<T>::emit(new_value);
            }
     };
}

#endif