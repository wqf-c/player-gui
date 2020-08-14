#include "PlayerApp.h"


/**
 * ³ÌÐòÈë¿Ú
 */


#ifdef _WIN32

#if NOT_USE_CONSOLE

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#endif //NOT_USE_CONSOLE

int main() {
	PlayerApp *app = new PlayerApp;
	wxApp::SetInstance(app);
	return wxEntry();
}
//	IMPLEMENT_APP_CONSOLE(TheiaApp);


#else

IMPLEMENT_APP(KankanApp);

#endif