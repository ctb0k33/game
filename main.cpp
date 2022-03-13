#include "CommonFunction.h"
#include "BaseObject.h"
#include"game_map.h"
#include"MainObject.h";
#include"ImpTimer.h"
#include"ThreatsObject.h"
BaseObject g_background;

bool InitData() {
	bool success = true;
	int ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		return false;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	g_window = SDL_CreateWindow(" SDL2.0 C++ Game", 
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								SCREEN_WIDTH, SCREEN_HEIGHT,
								SDL_WINDOW_SHOWN);
	if (g_window == NULL) {
		success= false;
	}
	else {
		g_screen = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
		if (g_screen == NULL) {
			success = false;
		}
		else {
			SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
			int imgFlags = IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) && imgFlags)) {
				success = false;
			}
		}
	}
	return success;
}
bool LoadBackground() {
	bool ret = g_background.LoadImg("Debug//image//loaded.png", g_screen);
	if (ret == false) {
		return false;
	}
	return true;
}
void close() {
	g_background.Free();

	SDL_DestroyRenderer(g_screen);
	g_screen = NULL;
	SDL_DestroyWindow(g_window);
	g_window = NULL;

	IMG_Quit();
	SDL_Quit();
}

std::vector<ThreatsObject*> MakeThreadList() {
	std::vector<ThreatsObject*> list_threats;

	ThreatsObject* dynamic_threats = new ThreatsObject[20];
	for (int i = 0; i < 20; i++) {
		ThreatsObject* p_threat = (dynamic_threats + i);
		if (p_threat != NULL) {
			p_threat->LoadImg("Debug//image//threat_left.png", g_screen);
			p_threat->Set_clips();
			p_threat->set_type_move(ThreatsObject::MOVE_IN_SPACE_THREAT);
			p_threat->set_x_pos(500 + i * 500);
			p_threat->set_y_pos(200);

			int pos1 = p_threat->get_x_pos() - 60;
			int pos2 = p_threat->get_x_pos() + 60;
			p_threat->SetAnimationPos(pos1, pos2);
			p_threat->set_input_left(1);
			list_threats.push_back(p_threat);
		}
	}

	ThreatsObject* threats_objs = new ThreatsObject[20];

	for (int i = 0; i < 20; i++) {
		ThreatsObject* p_threat = (threats_objs + i);
		if (p_threat != NULL) {
			p_threat->LoadImg("Debug//image//threat_level.png", g_screen);
			p_threat->Set_clips();
			p_threat->set_x_pos(700+i*1200);
			p_threat->set_y_pos(250);
			p_threat->set_type_move(ThreatsObject::STATIC_THREAT);
			p_threat->set_input_left(0);

			BulletObject* p_bullet = new BulletObject();
			p_threat->InitBullet(p_bullet, g_screen);
			list_threats.push_back(p_threat);
		}
	}

	return list_threats;
}
int main(int argc, char* agrv[]) {

	ImpTimer fps_timer;

	if (InitData() == false)
		return -1;
	if (LoadBackground() == false)
		return -1;


	GameMap game_map;
	game_map.LoadMap("map/map good/map01.dat");
	game_map.LoadTiles(g_screen);


	MainObject p_player;
	p_player.LoadImg("Debug//image//player_right.png", g_screen);
	p_player.set_clips();


	std::vector<ThreatsObject*> threats_list = MakeThreadList();


	bool is_quit = false;
	while (!is_quit)
	{
		fps_timer.start();
		while (SDL_PollEvent(&g_event) != 0)
		{
			if (g_event.type == SDL_QUIT) {
				is_quit = true;
			}
			p_player.HandleInputAction(g_event, g_screen);
		}
		SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
		SDL_RenderClear(g_screen);

		g_background.Render(g_screen, NULL);


		Map map_data = game_map.getMap();

		p_player.HandleBullet(g_screen);

		p_player.SetMapXY(map_data.start_x_, map_data.start_y_);
		p_player.DoPlayer(map_data);
		p_player.Show(g_screen);

		game_map.SetMap(map_data);
		game_map.DrawMap(g_screen);


		for (int i = 0; i < threats_list.size(); i++) {
			ThreatsObject* p_threat = threats_list.at(i);
			if (p_threat != NULL) {
				p_threat->SetMapXY(map_data.start_x_, map_data.start_y_);
				p_threat->ImpMoveType(g_screen);
				p_threat->DoPlayer(map_data);
				p_threat->MakeBullet(g_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
				p_threat->Show(g_screen);

				SDL_Rect rect_player = p_player.GetRectFrame();
				bool bCol1 = false;
				std::vector<BulletObject*> tBullet_list = p_threat->get_bullet_list();
				for (int jj = 0; jj < tBullet_list.size(); ++jj) {
					BulletObject* pt_bullet = tBullet_list.at(jj);
					if (pt_bullet) {
						bCol1 = SDLCommonFunc::CheckCollision(pt_bullet->GetRect(), rect_player);
						if (bCol1) {
							p_threat->RemoveBulelt(jj);
							break;
						}
					}
				}
				//bCol1 = false;
				SDL_Rect rect_threat = p_threat->GetRectFrame();
				bool bCol2 = SDLCommonFunc::CheckCollision(rect_player, rect_threat);
				if (bCol1 || bCol2) {
					if (MessageBox(NULL, L"GAME OVER",L"Info",MB_OK | MB_ICONSTOP) == IDOK) {
						p_threat->Free();
						close();
						SDL_Quit();
						return 0;
					}
				}
			}
		}



		std::vector<BulletObject*> bullet_arr = p_player.get_bullet_list();
		for (int r = 0; r < bullet_arr.size(); r++) {
			BulletObject* p_bullet = bullet_arr.at(r);
			if (p_bullet != NULL) {
				for (int t = 0; t < threats_list.size(); t++) {
					ThreatsObject* obj_threat = threats_list.at(t);
					if (obj_threat != NULL) {
						SDL_Rect tRect;
						tRect.x = obj_threat->GetRect().x;
						tRect.y = obj_threat->GetRect().y;
						tRect.w = obj_threat->get_width_frame_();
						tRect.h = obj_threat->get_height_frame_();

						SDL_Rect bRect = p_bullet->GetRect();

						bool bCol = SDLCommonFunc::CheckCollision(bRect, tRect);
						if (bCol) {
							p_player.RemoveBulelt(r);
							obj_threat->Free();
							threats_list.erase(threats_list.begin() + t);
						}
					}
				}
			}
		}

		SDL_RenderPresent(g_screen);

		int real_imp_time = fps_timer.get_ticks();// lay thoi gian thuc te
		int time_one_frame = 1000 / FRAME_PER_SECOND; //ms

		if (real_imp_time < time_one_frame) {
			int delay_time = time_one_frame - real_imp_time;
			if (delay_time >= 0)
				SDL_Delay(delay_time);// tao do tre
		}
	}
	for (int i = 0; i < threats_list.size(); i++) {
		ThreatsObject* p_threat = threats_list.at(i);
		if (p_threat) {
			p_threat->Free();
			p_threat = NULL;
		}
	}
	threats_list.clear();

	close();
	return 0;
}
