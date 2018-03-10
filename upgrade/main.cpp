#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "bg.h"
#include "od.h"
#include "font.h"

extern uint8_t rw_bg_array[99477];
extern uint8_t rw_od_array[6483];
extern uint8_t rw_font_array[367112];

#define FONT_SIZE 16

enum install_type {
	YES, NO
};

struct package_item {
	std::string path;
	enum install_type install;
};

int selected_list=0;
int start_list=0;
SDL_Surface *display;
SDL_Surface *screen;
SDL_Surface *bg;
SDL_Surface *od;
TTF_Font *font;
std::vector<struct package_item> package_list;

void update_screen(void)
{
	SDL_SoftStretch(screen, NULL, display, NULL);
	SDL_Flip(display);
}

void set_progress(int val)
{
	float off;
	SDL_Rect rt;

  if(val > 100) {
    val = 100;
  }
  if(val < 0){
    val = 0;
  }

	off = (267.0 - 28.0) / 100.0;
  rt.x = 29;
  rt.y = 203;
  rt.w = 28 + (off * val);
  rt.h = 5;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0xcc, 0x00, 0x00));
	update_screen();
}

void clear_screen(void)
{
	SDL_Rect rt;
	rt.x = 29;
	rt.y = 27;
	rt.w = 267;
	rt.h = 181;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	update_screen();
}

void draw_target_icon(char *filename)
{
	SDL_Rect rt;

	rt.x = 218;
	rt.y = 37;
	SDL_Surface *p = IMG_Load(filename);
  SDL_BlitSurface(p, NULL, screen, &rt);
	SDL_FreeSurface(p);
	update_screen();
}

void draw_skeleton(void)
{
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color col={0xff, 0xff, 0xff};
	
	clear_screen();
	rt.x = 32;
	rt.y = 30;
	msg = TTF_RenderText_Solid(font, "A: Select/Unselect", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);

	rt.x = 32;
	rt.y = 50;
	msg = TTF_RenderText_Solid(font, "B: Select all/Unselect all", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);

	rt.x = 32;
	rt.y = 70;
	msg = TTF_RenderText_Solid(font, "Start: Upgrade OS", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	
	rt.x = 32;
	rt.y = 90;
	msg = TTF_RenderText_Solid(font, "Select: Exit app", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	
	rt.x = 29;
	rt.y = 110;
	rt.w = 267;
	rt.h = 5;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0xff, 0xff, 0x00));
	
	rt.x = 200;
	rt.y = 27;
	rt.w = 5;
	rt.h = 85;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0xff, 0xff, 0x00));
	update_screen();
}

void draw_logo(void)
{
	SDL_BlitSurface(od, NULL, screen, NULL);
	SDL_BlitSurface(bg, NULL, screen, NULL);
	update_screen();
}

void draw_package_list(void)
{
	int i, len, size=package_list.size();
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color cur_col;
	SDL_Color sel_col={0x00, 0x00, 0x80};
	SDL_Color unsel_col={0x00, 0x00, 0x00};
	SDL_Color text_col={0xff, 0xff, 0xff};
	
	len = (start_list + 4) > size ? size : start_list + 4;
	for(i=start_list; i<len; i++){
		rt.x = 29;
		rt.y = 120 + (i*21);
		rt.w = 267;
		rt.h = 21;
		cur_col = ((i == selected_list) ? sel_col : unsel_col);
		SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, cur_col.r, cur_col.g, cur_col.b));

		rt.x = 32;
		rt.y = 123 + (i*21);
		rt.w = 15;
		rt.h = 15;
		SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, text_col.r, text_col.g, text_col.b));
		if(package_list.at(i).install == NO){
			rt.x = 34;
			rt.y = 125 + (i*21);
			rt.w = 11;
			rt.h = 11;
			SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, cur_col.r, cur_col.g, cur_col.b));
		}
		
		rt.x = 52;
		rt.y = 120 + (i*21);
		msg = TTF_RenderText_Solid(font, package_list.at(i).path.c_str(), text_col);
		SDL_BlitSurface(msg, NULL, screen, &rt);
		SDL_FreeSurface(msg);
	}
	update_screen();
}

int main(int argc, char* argv[])
{
	int exit=0;
	SDL_Event event;

  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    printf("%s, failed to SDL_Init\n", __func__);
    return -1;
  }
  SDL_ShowCursor(0);
 
  display = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
  screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240, 16, 0, 0, 0, 0);
  if(screen == NULL){
    printf("%s, failed to SDL_SetVideMode\n", __func__);
    return -1;
  }
  if(TTF_Init() == -1){
    printf("failed to TTF_Init\n");
    return -1;
  }
	SDL_RWops *rw_font = SDL_RWFromMem(rw_font_array, sizeof(rw_font_array));
  font = TTF_OpenFontRW(rw_font, 1, FONT_SIZE);
	TTF_SetFontHinting(font, TTF_HINTING_MONO);
	TTF_SetFontOutline(font, 0);

	SDL_RWops *rw_bg = SDL_RWFromMem(rw_bg_array, sizeof(rw_bg_array));
	bg = IMG_Load_RW(rw_bg, 1);

	SDL_RWops *rw_od = SDL_RWFromMem(rw_od_array, sizeof(rw_od_array));
	od = IMG_Load_RW(rw_od, 1);

	package_list.reserve(1024);

	draw_logo();
	SDL_Delay(1500);
	draw_skeleton();

	package_item item;
	item.path = "test1";
	item.install = NO;
	package_list.push_back(item);
	
	item.path = "test2";
	item.install = YES;
	package_list.push_back(item);
	
	item.path = "test3";
	item.install = NO;
	package_list.push_back(item);
	
	item.path = "test4";
	item.install = YES;
	package_list.push_back(item);
	
	item.path = "test5";
	item.install = YES;
	package_list.push_back(item);
	
	draw_package_list();	
	while(!exit){
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_KEYDOWN){
        if(event.key.keysym.sym == SDLK_ESCAPE){
          exit = 1;
					break;
        }
        else if(event.key.keysym.sym == SDLK_LCTRL){
					package_list.at(selected_list).install = (package_list.at(selected_list).install == YES) ? NO : YES;
				}
        else if(event.key.keysym.sym == SDLK_LALT){
					int i, size=package_list.size();
					enum install_type type;

					type = package_list.at(selected_list).install == YES ? NO : YES;
					for(i=0; i<size; i++){
						package_list.at(i).install = type;
					}
				}
        else if(event.key.keysym.sym == SDLK_UP){
					if(selected_list > 0){
						selected_list-= 1;
					}
				}
        else if(event.key.keysym.sym == SDLK_DOWN){
					if(selected_list < package_list.size()){
						selected_list+= 1;
					}
				}
				draw_package_list();
      }   
    }
	}

  SDL_Quit();
	printf("task done !\n");
  return 0;    
}

