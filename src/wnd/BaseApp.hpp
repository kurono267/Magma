#pragma once

#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::perspective
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class MainApp;
struct GLFWKey;
struct GLFWMouse;
typedef std::shared_ptr<MainApp> spMainApp;

class BaseApp {
	public:
		BaseApp(spMainApp app) : mainApp(app){}
		virtual ~BaseApp(){}

		virtual bool init() = 0;
		virtual bool draw() = 0;
		virtual bool update() = 0;
		
		virtual bool onKey(const GLFWKey& key){
			return true;
		}
		virtual bool onMouse(const GLFWMouse& mouse){
			return true;
		}
		virtual bool onScroll(const glm::vec2& offset){
			return true;
		}
		virtual bool onExit() = 0;

		spMainApp mainApp;
};
typedef std::shared_ptr<BaseApp> spBaseApp;
