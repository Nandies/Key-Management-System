#ifndef APPLICATION_H
#define APPLICATION_H

#include "KeyManager.h"
#include "UserInterface.h"

// Main application class
class Application {
private:
	KeyManager keyManager;
	UserInterface userInterface;

public:
	Application();
	void run();
};

#endif // !APPLICATION_H
