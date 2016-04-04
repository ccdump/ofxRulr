#pragma once

#include "RigidBody.h"

#include "ofxRay.h"
#include <opencv2/calib3d/calib3d.hpp>

#define RULR_VIEW_DISTORTION_COEFFICIENT_COUNT 4
#define RULR_VIEW_CALIBRATION_FLAGS CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6 | CV_CALIB_ZERO_TANGENT_DIST
namespace ofxRulr {
	namespace Nodes {
		namespace Item {
			class View : public RigidBody {
			public:
				View(bool hasDistortion = true);
				virtual string getTypeName() const override;

				void init();
				void update();
				void drawObject();

				void setWidth(float);
				void setHeight(float);
				float getWidth() const;
				float getHeight() const;

				void setIntrinsics(cv::Mat cameraMatrix, cv::Mat distortionCoefficients = cv::Mat::zeros(RULR_VIEW_DISTORTION_COEFFICIENT_COUNT, 1, CV_64F));
				void setProjection(const ofMatrix4x4 &);

				cv::Size getSize() const;
				cv::Mat getCameraMatrix() const;
				virtual bool getHasDistortion() const { return this->hasDistortion; };
				cv::Mat getDistortionCoefficients() const;

				const ofxRay::Camera & getViewInObjectSpace() const;
				ofxRay::Camera getViewInWorldSpace() const;
			protected:
				void markViewDirty();

				void exportViewMatrix();
				void exportRayCamera();
				void exportYaml();

				ofParameter<float> focalLengthX, focalLengthY;
				ofParameter<float> principalPointX, principalPointY;

				const bool hasDistortion;
				ofParameter<float> distortion[RULR_VIEW_DISTORTION_COEFFICIENT_COUNT];

				//Versions of this view as an ofxRay::Camera in world space and object space
				ofxRay::Camera viewInObjectSpace;
				
				bool viewIsDirty = false;
			private:
				void rebuildView();
				void parameterCallback(float &);
				void serialize(Json::Value &);
				void deserialize(const Json::Value &);
				void populateInspector(ofxCvGui::InspectArguments &);

				ofxRay::Camera * testCamera;
			};
		}
	}
}