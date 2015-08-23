#include "Patch.h"
#include "ofxAssets.h"
#include "ofSystemUtils.h"
#include "ofxClipboard.h"

#include "ofxCvGui/Widgets/Button.h"

using namespace ofxCvGui;

namespace ofxRulr {
	namespace Graph {
		namespace Editor {
#pragma mark View
			//----------
			Patch::View::View(Patch & owner) :
				patchInstance(owner) {
				this->canvasElements->onUpdate += [this](ofxCvGui::UpdateArguments & args) {
					auto newLink = this->patchInstance.newLink;
					if (newLink) {
						auto nodeUnderCursor = this->getNodeHostUnderCursor(this->lastCursorPositionInCanvas);
						newLink->setSource(nodeUnderCursor);
						newLink->setCursorPosition(this->lastCursorPositionInCanvas);
					}
				};
				this->canvasElements->onMouse += [this](ofxCvGui::MouseArguments & args) {
					this->lastCursorPositionInCanvas = args.local;
				};
				this->canvasElements->onDraw += [this](ofxCvGui::DrawArguments & args) {
					auto newLink = this->patchInstance.newLink;
					if (newLink) {
						newLink->draw(args);
					}
					auto selection = this->patchInstance.selection.lock();
					if (selection) {
						ofPushStyle();
						ofSetLineWidth(1.0f);
						ofNoFill();
						ofRect(selection->getBounds());
						ofPopStyle();
					}
					if (this->patchInstance.getNodeHosts().empty()) {
						ofxCvGui::Utils::drawText("Double click to add a new node...", args.localBounds, false);
					}
				};

				this->getCanvasElementGroup()->onDraw.addListener([this](ofxCvGui::DrawArguments & args) {
					this->drawGridLines();
				}, -1, this);

				this->onKeyboard += [this](ofxCvGui::KeyboardArguments & args) {
					if (args.action == ofxCvGui::KeyboardArguments::Action::Pressed) {
						if (args.key == OF_KEY_BACKSPACE || args.key == OF_KEY_DEL) {
							this->patchInstance.deleteSelection();
						}
						if (ofGetKeyPressed(OF_KEY_CONTROL)) {
							switch (args.key) {
							case 'x':
								this->patchInstance.cut();
								break; 
							case 'c':
								this->patchInstance.copy();
								break;
							case 'v':
								this->patchInstance.paste();
								break;
							}
						}
					}
				};

				// NODE BROWSER
				this->nodeBrowser = make_shared<NodeBrowser>();
				this->nodeBrowser->disable(); //starts as hidden
				this->nodeBrowser->addListenersToParent(this, true); // nodeBrowser goes on top of all elements (last listener)
				this->nodeBrowser->onNewNode += [this](shared_ptr<Nodes::Base> & node) {
					this->patchInstance.addNode(node, ofRectangle(this->birthLocation, this->birthLocation + ofVec2f(200, 100)));
					this->nodeBrowser->disable();
				};
				this->canvasElements->onMouse += [this](ofxCvGui::MouseArguments & args) {
					if (args.isDoubleClicked(this)) {
						this->birthLocation = args.local;
						this->nodeBrowser->enable();
						this->nodeBrowser->reset();
					}
				};
			}
			
			//----------
			void Patch::View::resync() {
				this->canvasElements->clear();
				const auto & nodeHosts = this->patchInstance.getNodeHosts();
				for (const auto & it : nodeHosts) {
					this->canvasElements->add(it.second);
				}

				const auto & linkHosts = this->patchInstance.getLinkHosts();
				for (const auto & it : linkHosts) {
					this->canvasElements->add(it.second);
				}

				auto newLink = this->patchInstance.getNewLink();
				if (newLink) {
					this->canvasElements->add(newLink);
				}
			}

			//----------
			void Patch::View::drawGridLines() {
				ofPushStyle();
				ofNoFill();
				ofSetLineWidth(3.0f);
				ofSetColor(80);
				ofRect(this->canvasExtents);

				const int stepMinor = 20;
				const int stepMajor = 100;

				auto canvasTopLeft = this->canvasExtents.getTopLeft();
				auto canvasBottomRight = this->canvasExtents.getBottomRight();
				for (int x = 0; x < canvasBottomRight.x; x += stepMinor) {
					if (x == 0) {
						ofSetLineWidth(3.0f);
					}
					else if (x % stepMajor == 0) {
						ofSetLineWidth(2.0f);
					}
					else {
						ofSetLineWidth(1.0f);
					}
					ofLine(x, canvasTopLeft.y, x, canvasBottomRight.y);
				}
				for (int x = 0; x > canvasTopLeft.x; x -= stepMinor) {
					if (x == 0) {
						ofSetLineWidth(3.0f);
					}
					else if ((-x) % stepMajor == 0) {
						ofSetLineWidth(2.0f);
					}
					else {
						ofSetLineWidth(1.0f);
					}
					ofLine(x, canvasTopLeft.y, x, canvasBottomRight.y);
				}
				for (int y = 0; y < canvasBottomRight.y; y += stepMinor) {
					if (y == 0) {
						ofSetLineWidth(3.0f);
					}
					else if (y % stepMajor == 0) {
						ofSetLineWidth(2.0f);
					}
					else {
						ofSetLineWidth(1.0f);
					}
					ofLine(canvasTopLeft.x, y, canvasBottomRight.x, y);
				}
				for (int y = 0; y > canvasTopLeft.y; y -= 10) {
					if (y == 0) {
						ofSetLineWidth(3.0f);
					}
					else if ((-y) % stepMajor == 0) {
						ofSetLineWidth(2.0f);
					}
					else {
						ofSetLineWidth(1.0f);
					}
					ofLine(canvasTopLeft.x, y, canvasBottomRight.x, y);
				}

				ofPopStyle();
			}

			//----------
			shared_ptr<NodeHost> Patch::View::getNodeHostUnderCursor(const ofVec2f & cursorInCanvas) {
				shared_ptr<NodeHost> nodeUnderCursor;
				for (shared_ptr<Element> element : this->getCanvasElementGroup()->getElements()) {
					auto asNodeHost = dynamic_pointer_cast<NodeHost>(element);
					if (asNodeHost) {
						if (element->getBounds().inside(cursorInCanvas)) {
							nodeUnderCursor = asNodeHost;
						}
					}
				}
				return nodeUnderCursor;
			}

			//----------
			shared_ptr<NodeHost> Patch::View::getNodeHostUnderCursor() {
				return this->getNodeHostUnderCursor(this->lastCursorPositionInCanvas);
			}

			//----------
			const ofxCvGui::PanelPtr Patch::View::findScreen(const ofVec2f & xy, ofRectangle & currentPanelBounds) {
				auto xyLocal = xy - currentPanelBounds.getTopLeft();

				auto nodeUnderCursor = this->getNodeHostUnderCursor();
				if (nodeUnderCursor) {
					return nodeUnderCursor->getNodeInstance()->getView(); // also this will return PanelPtr() if no screen available
				}

				return ofxCvGui::PanelPtr();
			}

#pragma mark Patch
			//----------
			Patch::Patch() {
				RULR_NODE_INIT_LISTENER;
			}

			//----------
			Patch::~Patch() {
			}

			//----------
			string Patch::getTypeName() const {
				return "Patch";
			}

			//----------
			void Patch::init() {
				this->view = MAKE(View, *this);

				RULR_NODE_UPDATE_LISTENER;
				RULR_NODE_SERIALIZATION_LISTENERS;
				RULR_NODE_INSPECTOR_LISTENER;
			}

			//----------
			void Patch::serialize(Json::Value & json) {
				//Serialize the nodes
				auto & nodesJson = json["Nodes"];
				for (auto & nodeHost : this->nodeHosts) {
					auto & nodeHostJson = nodesJson[ofToString(nodeHost.first)];

					nodeHost.second->serialize(nodeHostJson);

					//serialize the ID seperately (since the nodeHost doesn't know this information)
					nodeHostJson["ID"] = nodeHost.first;
				}

				//Serialize the links
				for (auto & nodeJson : nodesJson) {
					auto & inputPinsJson = nodeJson["InputsPins"];

					auto nodeHost = this->nodeHosts[nodeJson["ID"].asInt()];
					auto node = nodeHost->getNodeInstance();

					for (auto & input : node->getInputPins()) {
						auto & inputPinJson = inputPinsJson[input->getName()];
						if (input->getIsExposedThroughParentPatch()) {
							inputPinJson["Exposed"] = true;
						}
						if (input->isConnected()) {
							auto linkSource = input->getConnectionUntyped();
							auto linkSourceIndex = linkSource->getNodeHost()->getIndex();
							inputPinJson["SourceNode"] = linkSourceIndex;
						}
						else {
							inputPinJson["SourceNode"] = Json::Value();
						}
					}
				}

				auto & canvasJson = json["Canvas"];
				canvasJson["Scroll"] << this->view->getScrollPosition();
			}

			//----------
			void Patch::deserialize(const Json::Value & json) {
				this->nodeHosts.clear();
				
				this->insertPatchlet(json, false);

				const auto & canvasJson = json["Canvas"];
				ofVec2f canvasScrollPosition;
				canvasJson["Scroll"] >> canvasScrollPosition;
				this->view->setScrollPosition(canvasScrollPosition);

			}

			//----------
			void Patch::insertPatchlet(const Json::Value & json, bool useNewIDs, ofVec2f offset) {
				bool hasOffset = offset != ofVec2f();
				map<int, int> reassignIDs;

				const auto & nodesJson = json["Nodes"];
				//Deserialise nodes
				for (const auto & nodeJson : nodesJson) {
					auto ID = (NodeHost::Index) nodeJson["ID"].asInt();
					if (useNewIDs) {
						//use a new ID instead, store a reference to what we changed
						//new ID's are generally used for paste operation
						auto newID = this->getNextFreeNodeHostIndex();
						reassignIDs.insert(pair<int, int>(ID, newID));
						ID = newID;
					}
					try {
						auto nodeHost = FactoryRegister::X().make(nodeJson, this);
						if (hasOffset) {
							auto bounds = nodeHost->getBounds();
							bounds.x += offset.x;
							bounds.y += offset.y;
							nodeHost->setBounds(bounds);
						}
						this->addNodeHost(nodeHost, ID);
					}
					RULR_CATCH_ALL_TO_ERROR
				}



				//Deserialise links into the nodes
				for (const auto & nodeJson : nodesJson) {
					auto ID = (NodeHost::Index) nodeJson["ID"].asInt();
					if (useNewIDs) {
						ID = reassignIDs.at(ID);
					}

					auto nodeHost = this->getNodeHost(ID);
					if (nodeHost) { // e.g. if this node has been deleted then link needs to be ignored
						auto node = nodeHost->getNodeInstance();
						const auto & inputPinsJson = nodeJson["InputsPins"];

						//go through all the input pins
						for (auto & inputPin : node->getInputPins()) {
							const auto & inputPinJson = inputPinsJson[inputPin->getName()];

							if (!inputPinJson.isNull()) { //check this pin has been serialised
								if (inputPinJson["SourceNode"].isNull()) {
									//no connection
									continue;
								}

								Patch * patchWhereLinkIsMade;
								
								//check if it's been exposed, in which case connection happens in host patch
								auto inputPinExposedJson = inputPinJson["Exposed"];
								if (!inputPinExposedJson.isNull() && inputPinExposedJson.asBool()) {
									//this pin has been exposed to the parent patch
									inputPin->setIsExposedThroughParentPatch(true);
									//the expoed pin may then also be connected in the parent patch
									patchWhereLinkIsMade = this->getParentPatch();
								}
								else {
									patchWhereLinkIsMade = this;
								}

								//get the source ID. WARNING : doesn't support paste operations + exposed pins
								auto sourceNodeHostIndex = (NodeHost::Index) inputPinJson["SourceNode"].asInt();
								if (useNewIDs) {
									if (patchWhereLinkIsMade != this) {
										throw(Exception("Patch::insertPatchlet : connecting exposed links isn't currently supported for reassigned ID's"));
									}
									sourceNodeHostIndex = reassignIDs.at(sourceNodeHostIndex);
								}
								
								//make the connnection (actually make it later)
								patchWhereLinkIsMade->connectPin(inputPin, sourceNodeHostIndex, true);
							}
						}
					}
				}

				//we rebuild link hosts later since we're using delayed connections
				this->view->resync();
			}

			//----------
			ofxCvGui::PanelPtr Patch::getView() {
				return this->view;
			}

			//----------
			void Patch::update() {
				//update selection to whatever is being inspected
				this->selection.reset();
				for (auto nodeHost : this->nodeHosts) {
					if (ofxCvGui::isBeingInspected(nodeHost.second->getNodeInstance())) {
						this->selection = nodeHost.second;
						break;
					}
				}

				//deal with delayed connections
				if (!this->delayedConnections.empty()) {
					for (const auto & delayedConnection : this->delayedConnections) {
						auto pin = delayedConnection.first.lock();
						if (pin) {
							try {
								this->connectPin(pin, delayedConnection.second, false);
							}
							RULR_CATCH_ALL_TO_ERROR;
						}
					}
					this->delayedConnections.clear();
					this->rebuildLinkHosts();
				}
			}

			//----------
			void Patch::drawWorld() {
				for (auto nodeHost : this->nodeHosts) {
					nodeHost.second->getNodeInstance()->drawWorld();
				}
			}

			//----------
			void Patch::rebuildLinkHosts() {
				this->linkHosts.clear();
				for (auto targetNodeHost : this->nodeHosts) {
					auto targetNode = targetNodeHost.second->getNodeInstance();

					//build all standard links
					for (auto targetPin : targetNode->getInputPins()) {
						if (targetPin->isConnected()) {
							auto sourceNode = targetPin->getConnectionUntyped();
							auto sourceNodeHost = this->findNodeHost(sourceNode);
							if (sourceNodeHost) {
								auto observedLinkHost = make_shared<ObservedLinkHost>(sourceNodeHost, targetNodeHost.second, targetPin);
								auto linkHost = dynamic_pointer_cast<LinkHost>(observedLinkHost);
								this->linkHosts.insert(pair <LinkHost::Index, shared_ptr<LinkHost>>(this->getNextFreeLinkHostIndex(), linkHost));
							}
						}
					}

					//build all links as exposed
					auto nodeAsPatch = dynamic_pointer_cast<Patch>(targetNode);
					if (nodeAsPatch) {
						//it's a patch, and might have exposed pins we need to handle
						const auto & exposedInputs = nodeAsPatch->getExposedPins();
						for (auto exposedInput : exposedInputs) {
							auto targetPin = exposedInput.first.lock();
							if (targetPin) {
								auto sourceNode = targetPin->getConnectionUntyped();
								if (sourceNode) {
									//ok there's a connection
									auto sourceNodeHost = sourceNode->getNodeHost()->shared_from_this();
									auto observedLinkHost = make_shared<ObservedLinkHost>(sourceNodeHost, targetNodeHost.second, targetPin);
									auto linkHost = dynamic_pointer_cast<LinkHost>(observedLinkHost);
									this->linkHosts.insert(pair <LinkHost::Index, shared_ptr<LinkHost>>(this->getNextFreeLinkHostIndex(), linkHost));
								}
								
							}
						}
					}
				}
			}

			//----------
			const Patch::NodeHostSet & Patch::getNodeHosts() const {
				return this->nodeHosts;
			}

			//----------
			const Patch::LinkHostSet & Patch::getLinkHosts() const {
				return this->linkHosts;
			}

			//----------
			shared_ptr<NodeHost> Patch::addNode(NodeHost::Index index, shared_ptr<Nodes::Base> node, const ofRectangle & bounds) {
				auto nodeHost = make_shared<NodeHost>(node);
				if (bounds != ofRectangle()) {
					nodeHost->setBounds(bounds);
				}
				this->addNodeHost(nodeHost, index);
				return nodeHost;
			}

			//----------
			shared_ptr<NodeHost> Patch::addNode(shared_ptr<Nodes::Base> node, const ofRectangle & bounds) {
				return this->addNode(this->getNextFreeNodeHostIndex(), node, bounds);
			}

			//----------
			shared_ptr<NodeHost> Patch::addNewNode(shared_ptr<BaseFactory> factory, const ofRectangle & bounds) {
				auto newNode = factory->makeUntyped();
				newNode->init();
				return this->addNode(newNode, bounds);
			}

			//----------
			void Patch::addNodeHost(shared_ptr<ofxRulr::Graph::Editor::NodeHost> nodeHost, int index) {
				//all add's go through here or FactoryRegister::add
				auto node = nodeHost->getNodeInstance();
				node->setParentPatch(this);
				nodeHost->setIndex(index);

				weak_ptr<NodeHost> nodeHostWeak = nodeHost;
				nodeHost->onBeginMakeConnection += [this, nodeHostWeak](const shared_ptr<AbstractPin> & inputPin) {
					auto nodeHost = nodeHostWeak.lock();
					if (nodeHost) {
						this->callbackBeginMakeConnection(nodeHost, inputPin);
					}
				};
				nodeHost->onReleaseMakeConnection += [this](ofxCvGui::MouseArguments & args) {
					this->callbackReleaseMakeConnection(args);
				};
				nodeHost->onDropInputConnection += [this](const shared_ptr<AbstractPin> &) {
					this->view->resync();
				};
				nodeHost->getNodeInstance()->onAnyInputConnectionChanged += [this]() {
					this->rebuildLinkHosts();
				};

				this->nodeHosts.insert(pair<NodeHost::Index, shared_ptr<NodeHost>>(index, nodeHost));

				this->view->resync();
			}
			
			//----------
			void Patch::addNodeHost(shared_ptr<ofxRulr::Graph::Editor::NodeHost> nodeHost) {
				this->addNodeHost(nodeHost, this->getNextFreeNodeHostIndex());
			}
			
			//----------
			void Patch::deleteSelection() {
				auto selection = this->selection.lock();
				for (auto nodeHost : this->nodeHosts) {
					if (nodeHost.second == selection) {
						this->nodeHosts.erase(nodeHost.first);
						break;
					}
				}
				this->rebuildLinkHosts();
				this->view->resync();
			}

			//----------
			void Patch::cut() {
				if (!this->selection.expired()) {
					this->copy();
					this->deleteSelection();
				}
			}

			//----------
			void Patch::copy() {
				auto selection = this->selection.lock();
				if (selection) {
					//get the json
					Json::Value json;
					selection->serialize(json);

					//push to clipboard
					stringstream jsonString;
					Json::StyledStreamWriter styledWriter;
					styledWriter.write(jsonString, json);
					ofxClipboard::copy(jsonString.str());
				}
			}

			//----------
			void Patch::paste() {
				//get the clipboard into json
				const auto clipboardText = ofxClipboard::paste();
				Json::Value json;
				Json::Reader().parse(clipboardText, json);

				//if we got something
				if (json.isObject()) {
					//insert it with an offset
					this->insertPatchlet(json, true, ofVec2f(20, 20));
				}
			}

			//----------
			shared_ptr<TemporaryLinkHost> Patch::getNewLink() const {
				return this->newLink;
			}

			//----------
			shared_ptr<NodeHost> Patch::findNodeHost(shared_ptr<Nodes::Base> node) const {
				for (auto nodeHost : this->nodeHosts) {
					if (nodeHost.second->getNodeInstance() == node) {
						return nodeHost.second;
					}
				}
				return shared_ptr<NodeHost>();
			}

			//----------
			shared_ptr<NodeHost> Patch::getNodeHost(NodeHost::Index index) const {
				shared_ptr<NodeHost> nodeHost;
				if (this->nodeHosts.find(index) != this->nodeHosts.end()) {
					nodeHost = this->nodeHosts.at(index);
				}
				return nodeHost; // Returns empty pointer if not available
			}

			//----------
			void Patch::exposePin(shared_ptr<AbstractPin> pin, Nodes::Base * node) {
				//remove it if we already have it
				this->unexposePin(pin);

				//we have a set of exposedPinSet and we add and remove them manually to inputPins without calling addInput which adds an input to the node itself

				auto inserter = pair<weak_ptr<AbstractPin>, ExposedPin>(pin, { pin, node });
				this->exposedPinSet.insert(inserter);
				this->onExposedPinsChanged.notifyListeners();
			}

			//----------
			void Patch::unexposePin(shared_ptr<AbstractPin> pin) {
				auto findPin = this->exposedPinSet.find(pin);
				if (findPin != this->exposedPinSet.end()) {
					this->exposedPinSet.erase(findPin);
					this->onExposedPinsChanged.notifyListeners();
				}
			}

			//----------
			const Patch::ExposedPinSet & Patch::getExposedPins() const {
				return this->exposedPinSet;
			}

			//----------
			void Patch::connectPin(shared_ptr<AbstractPin> pin, NodeHost::Index nodeHostIndex, bool delayConnection) {
				if (!delayConnection) {
					//perform now
					auto nodeHost = this->getNodeHost(nodeHostIndex);
					if (!nodeHost) {
						throw(Exception("Patch::connectPin : nodeHost with index [" + ofToString(nodeHostIndex) + "] not found"));
					}
					pin->connect(nodeHost->getNodeInstance()); 
				}
				else {
					//store to perform later
					pair<weak_ptr<AbstractPin>, NodeHost::Index> inserter;
					inserter.first = pin;
					inserter.second = nodeHostIndex;
					this->delayedConnections.insert(inserter);
				}
			}

			//----------
			bool Patch::isRootPatch() const {
				if (this->parentPatch) {
					return false;
				}
				else {
					return true;
				}
			}
			//----------
			void Patch::populateInspector(ofxCvGui::ElementGroupPtr inspector) {
				inspector->add(Widgets::Button::make("Duplicate patch down", [this]() {
					Json::Value json;
					this->serialize(json);
					this->insertPatchlet(json, true, this->view->getCanvasExtents().getBottomLeft());
				}));
				inspector->add(Widgets::Button::make("Duplicate patch right", [this]() {
					Json::Value json;
					this->serialize(json);
					this->insertPatchlet(json, true, this->view->getCanvasExtents().getTopRight());
				}));
			}

			//----------
			NodeHost::Index Patch::getNextFreeNodeHostIndex() const {
				if (this->nodeHosts.empty()) {
					return 0;
				}
				else {
					return this->nodeHosts.rbegin()->first + 1;
				}
			}

			//----------
			LinkHost::Index Patch::getNextFreeLinkHostIndex() const {
				if (this->linkHosts.empty()) {
					return 0;
				}
				else {
					return this->linkHosts.rbegin()->first + 1;
				}
			}

			//----------
			void Patch::callbackBeginMakeConnection(shared_ptr<NodeHost> targetNodeHost, shared_ptr<AbstractPin> targetPin) {
				if (targetPin->isVisibleInPatch(this)) {
					//only make the newLink if it's supposed to be visible in this patch
					this->newLink = make_shared<TemporaryLinkHost>(targetNodeHost, targetPin);
				}
			}

			//----------
			void Patch::callbackReleaseMakeConnection(ofxCvGui::MouseArguments & args) {
				if (args.button == 2) {
					//right click, clear the link
					this->newLink.reset();
					this->view->resync();
				}
				else if (args.button == 0) {
					if (this->newLink) {
						//left click and we're making the link in this patch, try and make the link
						//if newLink is blank, then chances are that we're making the link in the parent patch
						this->newLink->flushConnection(); // this will trigger a notice downstream to rebuild the list of connections

														  //clear the temporary link regardless of success
						this->newLink.reset();
						this->view->resync();
					}
				}
			}
		}
	}
}