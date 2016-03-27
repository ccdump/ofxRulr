#pragma once

#include "ofxRulr/Nodes/Procedure/Base.h"
#include "ofxCvGui/Panels/Groups/Grid.h"

namespace ofxRulr {
	namespace Nodes {
		namespace Procedure {
			namespace Calibrate {
				class CameraFromKinectV2 : public ofxRulr::Nodes::Procedure::Base {
				public:
					struct Correspondence {
						ofVec3f kinectObject;
						ofVec2f camera;
						ofVec2f cameraNormalized;
					};

					CameraFromKinectV2();
					string getTypeName() const override;
					void init();
					ofxCvGui::PanelPtr getPanel() override;
					void update();

					void serialize(Json::Value &);
					void deserialize(const Json::Value &);

					void addCapture();
					void calibrate();
				protected:
					void populateInspector(ofxCvGui::InspectArguments &);
					void drawWorld();
					void rebuildView();
					shared_ptr<ofxCvGui::Panels::Groups::Grid> view;

					ofParameter<bool> usePreTest;

					vector<Correspondence> correspondences;
					vector<ofVec2f> previewCornerFindsKinect;
					vector<ofVec2f> previewCornerFindsCamera;
					float error;
				};
			}
		}
	}
}