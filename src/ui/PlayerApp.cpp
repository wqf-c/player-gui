#include "PlayerApp.h"
#include "PlayerLivePage.h"



bool PlayerApp::OnInit() {
	auto* frame = new PlayerLivePage(nullptr);
	frame->Show(true);
	return true;
}