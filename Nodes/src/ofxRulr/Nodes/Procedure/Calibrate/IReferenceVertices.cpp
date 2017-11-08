#include "pch_RulrNodes.h"
#include "IReferenceVertices.h"

namespace ofxRulr {
	namespace Nodes {
		namespace Procedure {
			namespace Calibrate {
#pragma mark TargetVertex
				//----------
				void IReferenceVertices::Vertex::drawWorld(const ofColor & color) {
					//draw a 3D cross at the vertex position

					ofPushMatrix();
					ofTranslate(this->getWorldPosition());
					ofPushStyle();

					ofSetLineWidth(2.0f);
					ofSetColor(color);
					this->drawObjectLines();

					ofSetLineWidth(4.0f);
					ofSetColor(0);
					this->drawObjectLines();

					ofPopStyle();
					ofPopMatrix();
				}

				//----------
				void IReferenceVertices::Vertex::drawObjectLines() {
					ofPushStyle();
					{
						ofSetColor(this->isSelected() ? 255 : 100);
						ofDrawLine(-ofVec3f(0.1f, 0.0f, 0.0f), ofVec3f(0.1f, 0.0f, 0.0f));
						ofDrawLine(-ofVec3f(0.0f, 0.1f, 0.0f), ofVec3f(0.0f, 0.1f, 0.0f));
						ofDrawLine(-ofVec3f(0.0f, 0.0f, 0.1f), ofVec3f(0.0f, 0.0f, 0.1f));
					}
					ofPopStyle();
				}

#pragma mark ISelectTargetVertex
				//----------
				IReferenceVertices::IReferenceVertices() {
					RULR_NODE_INIT_LISTENER;
				}

				//----------
				string IReferenceVertices::getTypeName() const {
					return "Procedure::Calibrate::ISelectTargetVertex";
				}

				//----------
				void IReferenceVertices::init() {
					RULR_NODE_DRAW_WORLD_LISTENER;
				}

				//----------
				void IReferenceVertices::drawWorld() {
					auto vertices = this->getVertices();
					for (auto vertex : vertices) {
						vertex->drawWorld();
					}
				}

				//----------
				const vector<shared_ptr<IReferenceVertices::Vertex>> & IReferenceVertices::getVertices() const {
					return this->vertices;
				}
			}
		}
	}
}