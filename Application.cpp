#include "Application.h"

Application::Application() : keyManager(), userInterface(keyManager) {}

void Application::run() {
    userInterface.run();
}