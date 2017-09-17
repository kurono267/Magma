//
// Created by kurono267 on 18.09.17.
//

#include "wnd/MainApp.hpp"
#include "mtlpp.hpp"

#include <chrono>

using namespace mtlpp;

class TextureApp : public BaseApp {
	public:
		TextureApp(spMainApp app) : BaseApp(app) {}
		virtual ~TextureApp(){}

		bool init(){
			device = Device::CreateSystemDefaultDevice();

			return true;
		}
		bool draw(){

			return true;
		}
		bool update(){
			return true;
		}

		bool onKey(const GLFWKey& key){}
		bool onMouse(const GLFWMouse& mouse){}
		bool onExit(){

		}
	protected:
		Device device;
};

int main(){
	spMainApp main = MainApp::instance();
	spBaseApp app = std::make_shared<TextureApp>(main);

	main->create("Texture",1280,720);
	main->setBaseApp(app);

	main->run();

	return 0;
}