#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "bg.h"
#include "od.h"
#include "mv.h"
#include "sys.h"
#include "font.h"

extern uint8_t rw_mv_array[4476];
extern uint8_t rw_od_array[6483];
extern uint8_t rw_bg_array[99477];
extern uint8_t rw_sys_array[4297];
extern uint8_t rw_font_array[367112];

#define CFW_VERSION 		"CFW v1.1 (v20180313)"
#define MOUNT_POINT			"/mnt/tmp/"
#define CREATE_TMP			"mkdir -p " MOUNT_POINT
#define REMOVE_TMP			"rm -rf " MOUNT_POINT
#define MOUNT_PACKAGE		"mount /mnt/int_sd/cfw_1.2_package.ext3 " MOUNT_POINT
#define UMOUNT_PACKAGE	"umount " MOUNT_POINT
#define VERSION_PATH		MOUNT_POINT "version"
#define GAME_FOLDER			"/mnt/game/"
#define ROM_FOLDER			"/mnt/int_sd/"

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
SDL_Surface *mv;
SDL_Surface *sys;
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

void draw_install_icon(const char *filename)
{
	SDL_Rect rt;

	rt.x = 130;
	rt.y = 45;
	rt.w = 64;
	rt.h = 64;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	rt.x = 130;
	rt.y = 45;
	printf("icon: %s\n", filename);
	SDL_Surface *p = IMG_Load(filename);
  SDL_BlitSurface(p, NULL, screen, &rt);
	SDL_FreeSurface(p);
	update_screen();
}

void draw_target_icon(const char *filename)
{
	SDL_Rect rt;

	rt.x = 232;
	rt.y = 27;
	rt.w = 64;
	rt.h = 64;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	rt.x = 232;
	rt.y = 27;
	printf("icon: %s\n", filename);
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
	msg = TTF_RenderText_Solid(font, "A: Select/Unselect it", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);

	rt.x = 32;
	rt.y = 50;
	msg = TTF_RenderText_Solid(font, "B: Select/Unselect all", col);
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
	rt.w = 203;
	rt.h = 5;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0xff, 0xff, 0x00));
	
	rt.x = 227;
	rt.y = 27;
	rt.w = 5;
	rt.h = 85;
  SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0xff, 0xff, 0x00));
	update_screen();
}

void draw_logo(void)
{
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color col = {0xff, 0xff, 0x00};
	SDL_BlitSurface(od, NULL, screen, NULL);
	SDL_BlitSurface(bg, NULL, screen, NULL);
	
	rt.x = 80;
	rt.y = 180;
	msg = TTF_RenderText_Solid(font, CFW_VERSION, col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	update_screen();
}

void draw_package_list(void)
{
	char buf[64];
	int i, len, size=package_list.size();
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color cur_col;
	SDL_Color sel_col={0x00, 0x00, 0x80};
	SDL_Color unsel_col={0x00, 0x00, 0x00};
	SDL_Color text_col={0xff, 0xff, 0xff};

	if(size == 0){
		return;
	}
	
	len = (start_list + 4) > size ? size : start_list + 4;
	for(i=start_list; i<len; i++){
		rt.x = 29;
		rt.y = 120 + ((i - start_list) * 21);
		rt.w = 267;
		rt.h = 21;
		cur_col = ((i == selected_list) ? sel_col : unsel_col);
		SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, cur_col.r, cur_col.g, cur_col.b));

		rt.x = 32;
		rt.y = 123 + ((i - start_list) * 21);
		rt.w = 15;
		rt.h = 15;
		SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, text_col.r, text_col.g, text_col.b));
		rt.x = 34;
		rt.y = 125 + ((i - start_list) * 21);
		rt.w = 11;
		rt.h = 11;
		SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, cur_col.r, cur_col.g, cur_col.b));
		if(package_list.at(i).install == YES){
			rt.x = 36;
			rt.y = 127 + ((i - start_list) * 21);
			rt.w = 7;
			rt.h = 7;
			SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, text_col.r, text_col.g, text_col.b));
		}
		
		rt.x = 52;
		rt.y = 120 + ((i - start_list) * 21);
		msg = TTF_RenderText_Solid(font, package_list.at(i).path.c_str(), text_col);
		SDL_BlitSurface(msg, NULL, screen, &rt);
		SDL_FreeSurface(msg);

		if(i == selected_list){
			std::string path = MOUNT_POINT + package_list.at(i).path + "/icon.png";
			draw_target_icon(path.c_str());
		}
	}

	rt.x = 232;
	rt.y = 94;
	rt.w = 62;
	rt.h = 24;
	SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, unsel_col.r, unsel_col.g, unsel_col.b));
	len = 0;
	for(i=0; i<size; i++){
		if(package_list.at(i).install == YES){
			len+= 1;
		}
	}
	rt.x = 235;
	rt.y = 97;
	sprintf(buf, "%03d/%03d", len, size);
	msg = TTF_RenderText_Solid(font, buf, text_col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	update_screen();
}

void show_msgbox(int x, int y, const char *buf)
{
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color col={0xff, 0xff, 0xff};
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x80));
	
	rt.x = x;
	rt.y = y;
	msg = TTF_RenderText_Solid(font, buf, col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	update_screen();
}

int mount_package(void)
{
	system("mount -o remount,rw / /");
	system(CREATE_TMP);
	system(MOUNT_PACKAGE);

	char buf[64]={0};
	int fd = open(VERSION_PATH, O_RDONLY);
	if(fd > 0){
		read(fd, buf, 64);
		close(fd);
	}
	else{
		printf("failed to open: %s\n", VERSION_PATH);
		return -1;
	}

	printf("Ver: %s\n", buf);
	if(memcmp(buf, CFW_VERSION, sizeof(CFW_VERSION)-1) == 0){
		return 0;
	}
	return -1;
}

void umount_package(void)
{
	system(UMOUNT_PACKAGE);
	system(REMOVE_TMP);
}

int list_dir(void)
{
	int n;
	package_item item;
	struct dirent **namelist;

  n = scandir(MOUNT_POINT, &namelist, NULL, alphasort);
  if(n < 0){
  	printf("failed to opendir " MOUNT_POINT "\n");
		return -1;
	}
	
	while(n--){
		if((strcmp(namelist[n]->d_name, "version") == 0) ||
			(strcmp(namelist[n]->d_name, "gmenu2x") == 0) ||
			(strcmp(namelist[n]->d_name, "rootfs.tar.gz") == 0) ||
			(strcmp(namelist[n]->d_name, "lost+found") == 0) ||
			(strcmp(namelist[n]->d_name, ".") == 0) ||
			(strcmp(namelist[n]->d_name, "..") == 0))
		{
		}
		else{
			item.path = namelist[n]->d_name;
			item.install = YES;
			package_list.push_back(item);
		}
		free(namelist[n]);
	 }
	 free(namelist);
  return 0;
}

int migrate_folder(void)
{
	int n;
	char buf[128];
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color col = {0xff, 0xff, 0xff};
	struct dirent **namelist;

  n = scandir(GAME_FOLDER, &namelist, NULL, alphasort);
  if(n < 0){
  	printf("failed to opendir " GAME_FOLDER "\n");
		return -1;
	}
	
	while(n--){
		if((strcmp(namelist[n]->d_name, "gmenu2x") == 0) ||
			(strcmp(namelist[n]->d_name, "lost+found") == 0) ||
			(strcmp(namelist[n]->d_name, ".") == 0) ||
			(strcmp(namelist[n]->d_name, "..") == 0))
		{
		}
		else{
			clear_screen();
			rt.x = 130;
			rt.y = 45;
			SDL_BlitSurface(mv, NULL, screen, &rt);

			rt.x = 110;
			rt.y = 140;
			sprintf(buf, "Move folder:");
			msg = TTF_RenderText_Solid(font, buf, col);
			SDL_BlitSurface(msg, NULL, screen, &rt);
			SDL_FreeSurface(msg);
			
			rt.x = 110;
			rt.y = 160;
			sprintf(buf, ">> %s%s", GAME_FOLDER, namelist[n]->d_name);
			msg = TTF_RenderText_Solid(font, buf, col);
			SDL_BlitSurface(msg, NULL, screen, &rt);
			SDL_FreeSurface(msg);
			update_screen();

			sprintf(buf, "mv %s%s %s", GAME_FOLDER, namelist[n]->d_name, ROM_FOLDER);
			system(buf);
		}
		free(namelist[n]);
	 }
	 free(namelist);
  return 0;
}

void install_package(void)
{
	char buf[128];
	SDL_Rect rt;
	SDL_Surface *msg;
	SDL_Color col={0xff, 0xff, 0xff};
	int i, cnt=0, all=0, size=package_list.size();

	migrate_folder();
	for(i=0; i<size; i++){
		if(package_list.at(i).install == YES){
			all+= 1;
		}
	}
	for(i=0; i<size; i++){
		if(package_list.at(i).install == YES){
			clear_screen();
			
			cnt+= 1;
			rt.x = 110;
			rt.y = 140;
			sprintf(buf, "Install %03d/%03d:", cnt, all);
			msg = TTF_RenderText_Solid(font, buf, col);
			SDL_BlitSurface(msg, NULL, screen, &rt);
			SDL_FreeSurface(msg);
		
			rt.x = 110;
			rt.y = 160;
			std::string str = ">> " + package_list.at(i).path;
			msg = TTF_RenderText_Solid(font, str.c_str(), col);
			SDL_BlitSurface(msg, NULL, screen, &rt);
			SDL_FreeSurface(msg);
			
			std::string path = MOUNT_POINT + package_list.at(i).path + "/icon.png";
			draw_install_icon(path.c_str());
	
			sprintf(buf, "cp -a %s%s/* /mnt/", MOUNT_POINT, package_list.at(i).path.c_str());
			printf("cmd: %s\n", buf);
			system(buf);
			set_progress((cnt * 100) / all);
			SDL_Delay(1000);
		}
	}

	clear_screen();
	rt.x = 130;
	rt.y = 45;
	SDL_BlitSurface(sys, NULL, screen, &rt);
	rt.x = 90;
	rt.y = 140;
	msg = TTF_RenderText_Solid(font, "Upgrade GMenu2X...", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	update_screen();
	
	// remove exits files
	sprintf(buf, "rm %sgmenu2x/sections/emulators/fba*", MOUNT_POINT);
	printf("remove file: %s\n", buf);
	system(buf);
	sprintf(buf, "rm %sgmenu2x/sections/emulators/pcsx*", MOUNT_POINT);
	printf("remove file: %s\n", buf);
	system(buf);
	sprintf(buf, "rm %sgmenu2x/sections/games/sdlpal*", MOUNT_POINT);
	printf("remove file: %s\n", buf);
	system(buf);
	sprintf(buf, "rm %sgmenu2x/sections/games/jinyong*", MOUNT_POINT);
	printf("remove file: %s\n", buf);
	system(buf);
		
	// copy -a gmenu2x
	sprintf(buf, "cp -a %sgmenu2x/* %sgmenu2x/", MOUNT_POINT, GAME_FOLDER);
	printf("copy gmenu2x: %s\n", buf);
	system(buf);

	clear_screen();
	rt.x = 130;
	rt.y = 45;
	SDL_BlitSurface(sys, NULL, screen, &rt);
	rt.x = 90;
	rt.y = 140;
	msg = TTF_RenderText_Solid(font, "Upgrade System...", col);
	SDL_BlitSurface(msg, NULL, screen, &rt);
	SDL_FreeSurface(msg);
	update_screen();
		
	// tar xvf rootfs.tar.gz
	sprintf(buf, "cd /;tar xvf %srootfs.tar.gz", MOUNT_POINT);
	printf("tar rootfs.tar.gz: %s\n", buf);
	system(buf);
	
	// remove icon.png
	sprintf(buf, "rm /mnt/icon.png");
	printf("remove icon: %s\n", buf);
	system(buf);

	show_msgbox(130, 110, "Enjoy !");
	SDL_Delay(1500);
}

int main(int argc, char* argv[])
{
	int exit=0, ret;
	SDL_Event event;

  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    printf("%s, failed to SDL_Init\n", __func__);
    return -1;
  }
  SDL_ShowCursor(0);
 
  display = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
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
  font = TTF_OpenFontRW(rw_font, 1, 16);
	TTF_SetFontHinting(font, TTF_HINTING_MONO);
	TTF_SetFontOutline(font, 0);

	SDL_RWops *rw_bg = SDL_RWFromMem(rw_bg_array, sizeof(rw_bg_array));
	bg = IMG_Load_RW(rw_bg, 1);

	SDL_RWops *rw_od = SDL_RWFromMem(rw_od_array, sizeof(rw_od_array));
	od = IMG_Load_RW(rw_od, 1);

	SDL_RWops *rw_mv = SDL_RWFromMem(rw_mv_array, sizeof(rw_mv_array));
	mv = IMG_Load_RW(rw_mv, 1);

	SDL_RWops *rw_sys = SDL_RWFromMem(rw_sys_array, sizeof(rw_sys_array));
	sys = IMG_Load_RW(rw_sys, 1);

	package_list.reserve(1024);
	show_msgbox(60, 110, "Mount upgrade package ...");
	ret = mount_package();
	SDL_Delay(1000);
	if(ret == -1){
		show_msgbox(50, 110, "Corrupted upgrade package !");
		SDL_Delay(3000);
		goto finally;
	}

	show_msgbox(60, 110, "Check upgrade package...");
	ret = list_dir();
	SDL_Delay(1000);
	if(ret == -1){
		show_msgbox(50, 110, "Corrupted upgrade package !");
		SDL_Delay(3000);
		goto finally;
	}

	draw_logo();
	SDL_Delay(1500);
	draw_skeleton();
	draw_package_list();	
	while(!exit){
		SDL_Delay(100);
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_KEYDOWN){
        if(event.key.keysym.sym == SDLK_ESCAPE){
          exit = 1;
					break;
        }
        else if(event.key.keysym.sym == SDLK_RETURN){
					install_package();
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
        else if(event.key.keysym.sym == SDLK_TAB){
					if((selected_list - 3) > 0){
						selected_list-= 3;
						if(selected_list < start_list){
							start_list-= 3;
						}
					}
					else{
						selected_list = 0;
						start_list = 0;
					}
				}
        else if(event.key.keysym.sym == SDLK_UP){
					if(selected_list > 0){
						selected_list-= 1;
						if(selected_list < start_list){
							start_list-= 1;
						}
					}
				}
        else if(event.key.keysym.sym == SDLK_BACKSPACE){
					int size = package_list.size();

					if(size > 0){
						if((selected_list + 3) < size){
							selected_list+= 3;
							if(selected_list >= (start_list + 4)){
								start_list+= 3;
							}
						}
						else{
							selected_list = size - 1;
							start_list = size - 4;
							if(start_list < 0){
								start_list = 0;
							}
						}
					}
				}
        else if(event.key.keysym.sym == SDLK_DOWN){
					int size = package_list.size();

					if((size > 0) && ((selected_list + 1) < size)){
						selected_list+= 1;
						if(selected_list >= (start_list + 4)){
							start_list+= 1;
						}
					}
				}
				draw_package_list();
      }   
    }
	}

finally:
	umount_package();
	system("reboot");
  SDL_Quit();
	printf("task done !\n");
  return 0;    
}

