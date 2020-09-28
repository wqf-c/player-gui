#include"RenderTimer.h"

RenderTimer::RenderTimer(wxPanel *page) {
	panel = page;
}


void RenderTimer::Notify() {
	//��ʱ������
	//��ȾͼƬ
	//
	isTimeRender = true;
	panel->Refresh(false);//����pageҳ����ػ��¼�
}

void RenderTimer::start() {
	wxTimer::Start(45);
}