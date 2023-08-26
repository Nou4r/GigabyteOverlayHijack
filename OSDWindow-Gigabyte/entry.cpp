#include "overlay.hpp"


int main()
{
	Overlay* overlay = new Overlay(L"Consolas", 18.0);

	if (!overlay->init())
		return 1;
	if (!overlay->startup_d2d())
		return 1;



	overlay->begin_scene();
	overlay->clear_scene();
	overlay->draw_text(10, 10, "github.com/HappyCatOfficial", D2D1::ColorF(D2D1::ColorF::White));
	overlay->end_scene();
}
