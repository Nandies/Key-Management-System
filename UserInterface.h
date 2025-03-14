#ifndef USERINFERFACE_H
#define USERINFERFACE_H

#include "KeyManager.h"

// UserInterface class to handle user interaction
class UserInterface {
private:
	KeyManager& keyManager;

	void displayMainMenu();

public:
	UserInterface(KeyManager& manager);
	void run();
};

#endif // !USERINFERFACE_HSERINFERFACE_H
