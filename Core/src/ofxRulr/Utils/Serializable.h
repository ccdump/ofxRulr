#pragma once

#include "ofParameter.h"
#include "ofxLiquidEvent.h"

#include <json/json.h>
#include <string>
#include <type_traits>

#define RULR_SERIALIZE_LISTENERS \
	this->onSerialize += [this](Json::Value & json) { \
		this->serialize(json); \
	}; \
	this->onDeserialize += [this](Json::Value const & json) { \
		this->deserialize(json); \
	}

namespace ofxRulr {
	namespace Utils {
		class Serializable {
		public:
			virtual std::string getTypeName() const = 0;
			virtual std::string getName() const;

			ofxLiquidEvent<Json::Value> onSerialize;
			ofxLiquidEvent<const Json::Value> onDeserialize;
			void serialize(Json::Value &);
			void deserialize(const Json::Value &);

			void save(std::string filename = "");
			void load(std::string filename = "");
			std::string getDefaultFilename() const;
		
			//////////////////////////////////////////////////////////////////////////

			template<typename Number>
			static void serializeNumber(Json::Value & json, const ofParameter<Number> & parameter) {
				const auto & value = parameter.get();
				if (value == value) { // don't serialize a NaN
					json[parameter.getName()] = parameter.get();
				}
			}
#define SERIALIZE_NUMBER(Type) static void serialize(Json::Value & json, const ofParameter<Type> & parameter) { serializeNumber(json, parameter); }
			SERIALIZE_NUMBER(uint8_t);
			SERIALIZE_NUMBER(uint16_t);
			SERIALIZE_NUMBER(uint32_t);
			SERIALIZE_NUMBER(uint64_t);
			SERIALIZE_NUMBER(int8_t);
			SERIALIZE_NUMBER(int16_t);
			SERIALIZE_NUMBER(int32_t);
			SERIALIZE_NUMBER(int64_t);
			SERIALIZE_NUMBER(bool);
			SERIALIZE_NUMBER(float);
			SERIALIZE_NUMBER(double);
			
			static void serialize(Json::Value &, const ofParameter<string> &);
			static void serialize(Json::Value &, const ofParameterGroup &);
			
			template<typename NotNumber,
				typename = enable_if<!is_arithmetic<NotNumber>::value, bool>::type>
			static void serialize(Json::Value & json, const ofParameter<NotNumber> & parameter) {
				json[parameter.getName()] << parameter.get();
			}

			//////////////////////////////////////////////////////////////////////////

			template<typename T>
			static void deserialize(const Json::Value & json, ofParameter<T> & parameter) {
				auto & jsonValue = json[parameter.getName()];
				if (!jsonValue.isNull()) {
					T value;
					jsonValue >> value;
					parameter.set(value);
				}
			}
			static void deserialize(const Json::Value &, ofParameter<int> &);
			static void deserialize(const Json::Value &, ofParameter<float> &);
			static void deserialize(const Json::Value &, ofParameter<bool> &);
			static void deserialize(const Json::Value &, ofParameter<string> &);
			static void deserialize(const Json::Value &, ofParameterGroup &);
		};
	}
}



//--
// Parameter groups
//--
//
// json >> parameterGroup; //deserialize
// json << parameterGroup; //serialize
template<typename T>
void operator>> (const Json::Value & json, ofParameterGroup & parameterGroup) {
	ofxRulr::Utils::Serializable::deserialize(json, parameterGroup);
}

template<typename T>
void operator<< (Json::Value & json, const ofParameterGroup & parameterGroup) {
	ofxRulr::Utils::Serializable::serialize(json, parameterGroup);
}
//
//--



//--
// Parameters
//--
//
// json >> parameter; //deserialize
// json << parameter; //serialize
template<typename T>
void operator>> (const Json::Value & json, ofParameter<T> & parameter) {
	ofxRulr::Utils::Serializable::deserialize(json, parameter);
}

template<typename T>
void operator<< (Json::Value & json, const ofParameter<T> & parameter) {
	ofxRulr::Utils::Serializable::serialize(json, parameter);
}
//
//--



//--
// Raw values
//--
//
// json >> value; //deserialize
// json << value; //serialize
//
// since the json object is useless after the stream, we return void
template<class T,
	typename = std::enable_if_t<!std::is_base_of<ofAbstractParameter, T>::value> >
void operator>> (const Json::Value & json, T & streamSerializableObject) {
	stringstream stream(json.asString());
	stream >> streamSerializableObject;
}

template<class T,
	typename = std::enable_if_t<!std::is_base_of<ofAbstractParameter, T>::value> >
	void operator<< (Json::Value & json, const T & streamSerializableObject) {
	stringstream stream;
	stream << streamSerializableObject;
	json = stream.str();
}
//
//--