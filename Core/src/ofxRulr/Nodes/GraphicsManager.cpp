#include "pch_RulrCore.h"
#include "GraphicsManager.h"

//----------
OFXSINGLETON_DEFINE(ofxRulr::Nodes::GraphicsManager);

namespace ofxRulr {
	namespace Nodes {
		//----------
		GraphicsManager::GraphicsManager() {

		}

		//----------
		shared_ptr<ofImage> GraphicsManager::getIcon(const string & nodeTypeName) {
			auto findIcon = this->icons.find(nodeTypeName);
			if (findIcon == this->icons.end()) {
				//no icon yet, let's load it
				auto imageName = "ofxRulr::Nodes::" + nodeTypeName;

				pair<string, shared_ptr<ofImage>> inserter;
				if (!ofxAssets::hasImage(imageName)) {
					//the image file doesn't exist, let's use default icon
					imageName = "ofxRulr::Nodes::Default";
				}
				auto image = make_shared<ofImage>();
				image->clone(ofxAssets::image(imageName));
				inserter.second = image;
				this->icons.insert(inserter);
				return inserter.second;
			}
			else {
				return findIcon->second;
			}
		}

		//----------
		ofColor GraphicsManager::getColor(const string & nodeTypeName) {
			auto findColor = this->colors.find(nodeTypeName);
			if (findColor == this->colors.end()) {
				auto color = ofxCvGui::Utils::toColor(nodeTypeName);
				this->colors.emplace(nodeTypeName, color);
				return color;
			}
			else {
				return findColor->second;
			}
		}

		//----------
		void GraphicsManager::setIcon(const string & nodeTypeName, shared_ptr<ofImage> icon) {
			this->icons[nodeTypeName] = icon;
		}

		//----------
		void GraphicsManager::setColor(const string & nodeTypeName, const ofColor & color) {
			//see setIcon for notes
			auto & storedColor = this->colors[nodeTypeName];
			storedColor = color;
		}
	}
}