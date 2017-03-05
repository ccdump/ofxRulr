#include "pch_RulrCore.h"
#include "World.h"

#include "Summary.h"

#include "ofxRulr/Exception.h"
#include "ofxRulr/Utils/Initialiser.h"
#include "ofxRulr/Utils/Utils.h"
#include "ofxRulr/Version.h"

using namespace ofxCvGui;

OFXSINGLETON_DEFINE(ofxRulr::Graph::World);

namespace ofxRulr {
	namespace Graph {
		//-----------
		ofxCvGui::Controller * World::gui = 0;

		//-----------
		World::World() {

		}

		//-----------
		World::~World() {

		}

		//-----------
		void World::init(Controller & controller, bool enableSummaryView) {
			Utils::initialiser.checkInitialised();

			ofSetWindowTitle("Rulr v" + RULR_VERSION_STRING);



			//--
			// INIITALISE NODES
			//--
			//
			set<shared_ptr<Nodes::Base>> failedNodes;
			for (auto node : *this) {
				bool initSuccess = false;
				try
				{
					node->init();
					initSuccess = true;
				}
				RULR_CATCH_ALL_TO_ALERT

				if (!initSuccess) {
					failedNodes.insert(node);
				}
			}
			for (auto failedNode : failedNodes) {
				this->remove(failedNode);
			}
			//
			//--



			//--
			// SETUP GUI GRID
			//--
			//
			auto horizontalGroup = make_shared<Panels::Groups::Strip>(Panels::Groups::Strip::Direction::Horizontal);
			controller.add(horizontalGroup);
			horizontalGroup->setCellSizes({ -1, 300 });
			horizontalGroup->setHandlesEnabled(true);

			auto verticalGroup = make_shared<Panels::Groups::Strip>(Panels::Groups::Strip::Direction::Vertical);
			verticalGroup->setHandlesEnabled(true);
			horizontalGroup->add(verticalGroup);
			//
			//--



			//--
			// NODE GRID
			//--
			//
			auto nodeGrid = MAKE(ofxCvGui::Panels::Groups::Grid);
			verticalGroup->add(nodeGrid);
			for(auto node : *this) {
				auto nodePanel = node->getPanel();
				if (nodePanel) {
					nodeGrid->add(nodePanel);
				}
			}
			//
			//--



			//--
			// SUMMARY VIEW
			//
			if (enableSummaryView) {
				auto summary = make_shared<Summary>();
				summary->setWorld(*this);
				summary->init();
				summary->setName("World");
				this->add(summary); // we intentionally do this after building the Node grid
				verticalGroup->add(summary->getPanel());
			}
			//
			//--



			//--
			// INSPECTOR
			//--
			//
			auto inspector = ofxCvGui::Panels::makeInspector();
			horizontalGroup->add(inspector);
			inspector->setTitleEnabled(false);

			//whenever the inspector clears, setup default elements
			InspectController::X().onClear += [this] (InspectArguments & args) {
				auto inspector = args.inspector;

				inspector->add(new Widgets::LiveValueHistory("Application fps [Hz]", [] () {
					return ofGetFrameRate();
				}, true));

				auto saveAllButton = inspector->add(new Widgets::Button("Save all", [this]() {
					this->saveAll();
				}));
				saveAllButton->onDraw += [this](ofxCvGui::DrawArguments & args) {
					//show time since last save if >1min
					auto duration = chrono::system_clock::now() - this->lastSaveOrLoad;
					if (duration > chrono::minutes(1)) {
						ofxAssets::font(ofxCvGui::getDefaultTypeface(), 8).drawString(Utils::formatDuration(duration, true, true, false) + "[since last save]", 6, 27);
					}
				};

				/*
				HACK
				Until this doesn't result in a crash always let's remove
				inspector->add(new Widgets::Button("Load all", [this]() {
					this->loadAll();
				}));
				*/

				inspector->add(new Widgets::Spacer());
			};
			//
			//--



			//--
			// NODE VIEW CALLBACKS
			//--
			//
			for (auto node : *this) {
				auto nodePanel = node->getPanel();
				if (nodePanel) {
					//if we click inside the panel, (regardless of what takes the click), then inspect this node

					//we add the listener to be LATE (remember that the mouse stack is notified in reverse)
					//this ensures that anything nested is called first
					nodePanel->onMouse.addListener([node, this](MouseArguments & mouse) {
						if (mouse.action == ofxCvGui::MouseArguments::Action::Pressed) {
							ofxCvGui::inspect(node);
						}
					}, this, -100);

					//draw outlines on gui panels if node is selected
					nodePanel->onDraw += [node](DrawArguments & drawArgs) {
						if (isBeingInspected(*node)) {
							ofPushStyle();
							ofSetColor(255);
							ofSetLineWidth(3.0f);
							ofNoFill();
							ofDrawRectangle(drawArgs.localBounds);
							ofPopStyle();
						}
					};
				}
			}
			//
			//--



			World::gui = & controller;
			
			if (!this->empty()) {
				ofxCvGui::inspect(this->front());
			}
		}

		//-----------
		void World::saveAll() const {
			for(auto node : * this) {
				node->save(node->getDefaultFilename());
			}
			const_cast<World*>(this)->lastSaveOrLoad = chrono::system_clock::now();
		}

		//-----------
		void World::loadAll(bool printDebug) {
			for(auto node : * this) {
				if (printDebug) {
					ofLogNotice("ofxRulr") << "Loading node [" << node->getName() << "]";
				}
				node->load(node->getDefaultFilename());
			}
			this->lastSaveOrLoad = chrono::system_clock::now();
		}

		//-----------
		ofxCvGui::Controller & World::getGuiController() {
			if (World::gui) {
				return * World::gui;
			} else {
				throw(Exception("No gui attached yet"));
			}
		}

		//----------
		ofxCvGui::PanelGroupPtr World::getGuiGrid() const {
			return this->guiGrid;
		}
	}
}