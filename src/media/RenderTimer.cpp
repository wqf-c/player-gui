#include"RenderTimer.h"

RenderTimer::RenderTimer(wxPanel *page) {
	panel = page;
}


void RenderTimer::Notify() {
	//��ʱ������
	//��ȾͼƬ
	//
	isTimeRender = true;
	panel->Refresh();//����pageҳ����ػ��¼�
}

void RenderTimer::start() {
	wxTimer::Start(45);
}