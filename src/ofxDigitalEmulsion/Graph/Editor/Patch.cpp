#include "Patch.h"
#include "ofxAssets.h"
#include "ofSystemUtils.h"

#include "ofxCvGui/Utils/Button.h"

namespace ofxDigitalEmulsion {
	namespace Graph {
		namespace Editor {
#pragma mark View
			//----------
			Patch::View::View(Patch & owner) :
				patchInstance(owner) {
				auto addButton = std::make_shared<ofxCvGui::Utils::Button>();
				addButton->setCaption("add node");
				addButton->onDrawUp += [](ofxCvGui::DrawArguments & args) {
					ofPushStyle();
					ofSetColor(255);
					ofxAssets::image("ofxDigitalEmulsion::plus").draw(args.localBounds);
					ofPopStyle();
				};
				addButton->onDrawDown += [](ofxCvGui::DrawArguments & args) {
					ofPushStyle();
					ofSetColor(100);
					ofxAssets::image("ofxDigitalEmulsion::plus").draw(args.localBounds);
					ofPopStyle();
				};
				this->fixedElements->add(addButton);
				
				addButton->onMouse += [this, addButton](ofxCvGui::MouseArguments & args) {
					if (args.action == ofxCvGui::MouseArguments::Action::Released) {
						try
						{
							this->patchInstance.addDebug();
						}
						OFXDIGITALEMULSION_CATCH_ALL_TO_ALERT
					}
				};

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
				};

				this->getCanvasElementGroup()->onDraw.addListener([this](ofxCvGui::DrawArguments & args) {
					this->drawGridLines();
				}, -1, this);
				this->onBoundsChange += [this, addButton](ofxCvGui::BoundsChangeArguments & args) {
					addButton->setBounds(ofRectangle(100, 100, 100, 100));
				};
			}
			
			//----------
			void Patch::View::resync() {
				this->canvasElements->clear();
				const auto & nodeHosts = this->patchInstance.getNodeHosts();
				for (const auto & it : nodeHosts) {
					this->canvasElements->add(it.second);
				}

				this->patchInstance.rebuildLinkHosts();
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

#pragma mark Instance
			//----------
			string Patch::getTypeName() const {
				return "Patch";
			}

			//----------
			void Patch::init() {
				this->view = MAKE(View, *this);
			}

			//----------
			void Patch::serialize(Json::Value & json) {

			}

			//----------
			void Patch::deserialize(const Json::Value & json) {

			}

			//----------
			ofxCvGui::PanelPtr Patch::getView() {
				return this->view;
			}

			//----------
			void Patch::update() {

			}

			//----------
			void Patch::rebuildLinkHosts() {
				this->linkHosts.clear();
				for (auto targetNodeHost : this->nodeHosts) {
					auto targetNode = targetNodeHost.second->getNodeInstance();
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
			void Patch::addNode(NodeHost::Index index, shared_ptr<Node> node) {
				auto nodeHost = make_shared<NodeHost>(node);
				this->nodeHosts.insert(pair<NodeHost::Index, shared_ptr<NodeHost>>(index, nodeHost));
				nodeHost->onBeginMakeConnection += [this, nodeHost](const shared_ptr<BasePin> & inputPin) {
					this->callbackBeginMakeConnection(nodeHost, inputPin);
				};
				nodeHost->onReleaseMakeConnection += [this](const shared_ptr<BasePin> &) {
					this->callbackReleaseMakeConnection();
				};
				this->view->resync();
			}

			//----------
			void Patch::addNewNode(shared_ptr<BaseFactory> factory) {
				this->addNode(this->getNextFreeNodeHostIndex(), factory->make());
			}

			//----------
			void Patch::addDebug() {
				auto & factoryRegister = FactoryRegister::X();
				if (factoryRegister.empty()) {
					throw(Utils::Exception("FactoryRegister has no entires"));
				}
				else {
					//pick a different one each time
					static int index = 0;
					index %= factoryRegister.size();
					auto it = factoryRegister.begin();
					std::advance(it, index++);

					this->addNewNode(it->second);
				}
			}

			//----------
			shared_ptr<TemporaryLinkHost> Patch::getNewLink() const {
				return this->newLink;
			}

			//----------
			shared_ptr<NodeHost> Patch::findNodeHost(shared_ptr<Node> node) const {
				for (auto nodeHost : this->nodeHosts) {
					if (nodeHost.second->getNodeInstance() == node) {
						return nodeHost.second;
					}
				}
				return shared_ptr<NodeHost>();
			}

			//----------
			void Patch::populateInspector2(ofxCvGui::ElementGroupPtr) {

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
			void Patch::callbackBeginMakeConnection(shared_ptr<NodeHost> targetNodeHost, shared_ptr<BasePin> targetPin) {
				this->newLink = make_shared<TemporaryLinkHost>(targetNodeHost, targetPin);
			}

			//----------
			void Patch::callbackReleaseMakeConnection() {
				if (this->newLink->flushConnection()) {
					//we made a new connection

					//get rid of the temporary connection
					this->newLink.reset();

					//update the view
					this->view->resync();
				}
			}
		}
	}
}