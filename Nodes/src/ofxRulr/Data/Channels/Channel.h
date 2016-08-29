#pragma once

#include "ofParameter.h"

#include "Address.h"

#include "ofxLiquidEvent.h"

#include <string>
#include <map>
#include <memory>

using namespace std;

namespace ofxRulr {
	namespace Data {
		namespace Channels {
			class Channel : public enable_shared_from_this<Channel> {
			public:
				enum Type {
					Undefined,
					Bool,
					Int,
					Int32,
					Int64,
					UInt32,
					UInt64,
					Float,
					String,
					Vec3f,
					Vec4f,
					IntVector,
					Unknown
				};

				typedef map<string, shared_ptr<Channel>> Set;
				
				Channel(const string & name);

				const string & getName() const;
				Channel & getSubChannel(const string & name);
				
				Set & getSubChannels();
				const Set & getSubChannels() const;

				Channel & operator[](const string & subChannelName);
				Channel & operator[](const Address & address);

				template<typename Type>
				shared_ptr<ofParameter<Type>> getParameter() {
					return dynamic_pointer_cast<ofParameter<Type>>(this->parameter);
				}

				template<typename Type>
				shared_ptr<ofParameter<Type>> getParameter() const {
					return dynamic_pointer_cast<ofParameter<Type>>(this->parameter);
				}

				template<typename Type>
				const Type & getValue() const {
					return this->getParameter<Type>()->get();
				}

				shared_ptr<ofAbstractParameter> getParameterUntyped();

				Type getValueType() const;

				Channel & addSubChannel(const string & name);
				void removeSubChannel(const string & name);

				void setSubChannel(shared_ptr<Channel>);

				template<typename T>
				void operator=(T value) {
					auto parameter = dynamic_pointer_cast<ofParameter<T>>(this->parameter);
					if (!parameter) {
						parameter = make_shared<ofParameter<T>>();
						parameter->setName(this->name);
						this->parameter = parameter;
					}

					if (typeid(T) == typeid(bool)) {
						this->type = Type::Bool;
					}
					else if (typeid(T) == typeid(int)) {
						this->type = Type::Int;
					}
					else if (typeid(T) == typeid(int32_t)) {
						this->type = Type::Int32;
					}
					else if (typeid(T) == typeid(int64_t)) {
						this->type = Type::Int64;
					}
					else if (typeid(T) == typeid(uint32_t)) {
						this->type = Type::UInt32;
					}
					else if (typeid(T) == typeid(uint64_t)) {
						this->type = Type::UInt64;
					}
					else if (typeid(T) == typeid(float)) {
						this->type = Type::Float;
					}
					else if (typeid(T) == typeid(string)) {
						this->type = Type::String;
					}
					else if (typeid(T) == typeid(ofVec3f)) {
						this->type = Type::Vec3f;
					}
					else if (typeid(T) == typeid(ofVec4f)) {
						this->type = Type::Vec4f;
					}
					else if (typeid(T) == typeid(vector<int>)) {
						this->type = Type::IntVector;
					}
					else {
						this->type = Type::Unknown;
					}

					parameter->set(value);
				}

				void clear();

				ofxLiquidEvent<void> onHeirarchyChange;
			protected:
				const string name;
				Set subChannels;
				shared_ptr<ofAbstractParameter> parameter;
				Type type = Type::Undefined;
			};
		}
	}
}