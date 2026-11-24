#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include "utility.h"
#include "game.h"
#include "autoplayer.h"

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define C1 v3d(232, 186, 60)
#define C2 v3d(236, 247, 30)
#define HC v3d(131, 237, 24)
#define TC v3d(237, 91, 24)

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset, float scale) {
	int px, py, c;
	SDL_Rect s, d;
	int P =  8 * scale;
	s.w = 8;
	s.h = 8;
	d.w = P;
	d.h = P;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitScaled(charset, &s, screen, &d);
		x += P;
		text++;
	};
};

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y, float scale) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2 * scale;
	dest.y = y - sprite->h / 2 * scale;
	dest.w = sprite->w * scale;
	dest.h = sprite->h * scale;
	SDL_BlitScaled(sprite, NULL, screen, &dest);
};

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitScaled(sprite, NULL, screen, &dest);
};

int getColor(SDL_Surface* surface, int r, int g, int b) {
	return SDL_MapRGB(surface->format, r, g, b);
}

void drawPixel(SDL_Surface* surface, v2d pos, Uint32 color) {
	if (pos.x < 0 || pos.x >= SCREEN_WIDTH || pos.y < 0 || pos.y >= SCREEN_HEIGHT) return;
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + (int)pos.y * surface->pitch + (int)pos.x * bpp;
	*(Uint32*)p = color;
};

void drawLine(SDL_Surface* surface, v2d p1, v2d p2, Uint32 color) {
	v2d d(p2.x - p1.x, p2.y - p1.y);
	float L = d.length(), l = 0;
	d /= L;
	while (l < L) {
		drawPixel(surface, p1, color);
		l++;
		p1 = p1 + d;
	}
}
void limitV2d(v2d& v) {
	if (v.x < 0) v.x = 0;
	else if (v.x >= SCREEN_WIDTH) v.x = SCREEN_WIDTH - 1;
	if (v.y < 0) v.y = 0;
	else if (v.y >= SCREEN_HEIGHT) v.y = SCREEN_HEIGHT - 1;
}

void drawScanline(SDL_Surface* surface, float x1, float x2, int y, Uint32 color) {
	if (x1 > x2) {
		float tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y < 0 || y >= SCREEN_HEIGHT) return;
	int l = x1 < 0 ? 0 : x1;
	int r = x2 >= SCREEN_WIDTH ? SCREEN_WIDTH - 1 : x2;
	for (int i = l; i <= r; i++) drawPixel(surface, v2d(i, y), color);
	if (false) {	//outline
		drawPixel(surface, v2d(l, y), getColor(surface, 0, 0, 0));
		drawPixel(surface, v2d(r, y), getColor(surface, 0, 0, 0));
	}
};

void swapV2d(v2d& p1, v2d& p2) {
	v2d tmp = p1;
	p1 = p2;
	p2 = tmp;
}

void drawTriangle(SDL_Surface* surface, v2d& p1, v2d& p2, v2d& p3, Uint32 color) {
	if (p1.y > p2.y) swapV2d(p1, p2);
	if (p2.y > p3.y) swapV2d(p2, p3);
	if (p1.y > p2.y) swapV2d(p1, p2);

	if (p2.y >= p1.y) {
		float invSlope1 = (p2.x - p1.x) / (p2.y - p1.y);
		float invSlope2 = (p3.x - p1.x) / (p3.y - p1.y);
		float curX1 = p1.x;
		float curX2 = p1.x;
		for (float y = p1.y; y < (p2.y); y++) {
			drawScanline(surface, curX1, curX2, y, color);
			curX1 += invSlope1;
			curX2 += invSlope2;
		}
	}

	if (p3.y >= p2.y) {
		float invSlope1 = (p3.x - p2.x) / (p3.y - p2.y);
		float invSlope2 = (p3.x - p1.x) / (p3.y - p1.y);
		float curX1 = p2.x;
		float curX2 = p3.x - (p3.y - p2.y) * invSlope2;
		for (float y = (p2.y); y < (p3.y); y++) {
			drawScanline(surface, curX1, curX2, y, color);
			curX1 += invSlope1;
			curX2 += invSlope2;
		}
	}
}


class model {
public:
	int pointsCount;
	int trisCount;
	v3d* points;
	v3d* tris;
	model(int n, int m) {
		pointsCount = n;
		trisCount = m;
		points = (v3d*)malloc(n * sizeof(v3d));
		tris = (v3d*)malloc(m * sizeof(v3d));
	}
	~model() { free(tris); free(points); }

	void scale(float k) {
		for (int i = 0; i < pointsCount; i++)
			points[i] *= k;
	}

	void move(v3d v) {
		for (int i = 0; i < pointsCount; i++)
			points[i] += v;
	}

	void createCube() {
		points[0] = v3d(-1, -1, -1);
		points[1] = v3d(1, -1, -1);
		points[2] = v3d(1, -1, 1);
		points[3] = v3d(-1, -1, 1);
		points[4] = v3d(-1, 1, -1);
		points[5] = v3d(1, 1, -1);
		points[6] = v3d(1, 1, 1);
		points[7] = v3d(-1, 1, 1);

		tris[0] = v3d(0, 2, 1);
		tris[1] = v3d(0, 3, 2);
		tris[2] = v3d(4, 6, 5);
		tris[3] = v3d(4, 7, 6);
		tris[4] = v3d(2, 6, 3);
		tris[5] = v3d(6, 7, 3);
		tris[6] = v3d(0, 5, 1);
		tris[7] = v3d(0, 4, 5);
		tris[8] = v3d(0, 7, 3);
		tris[9] = v3d(0, 4, 7);
		tris[10] = v3d(1, 6, 2);
		tris[11] = v3d(1, 5, 6);
		scale(0.5);
		move(v3d(1, 0.7, 1.5));
	}

	void createSnake() {
		points[0] = v3d(2, 0, 0);
		points[1] = v3d(8, 0, 0);
		points[2] = v3d(10, 0, 2);
		points[3] = v3d(10, 0, 8);
		points[4] = v3d(8, 0, 10);
		points[5] = v3d(2, 0, 10);
		points[6] = v3d(0, 0, 8);
		points[7] = v3d(0, 0, 2);
		points[8] = v3d(2, 0, 2);
		points[9] = v3d(8, 0, 2);
		points[10] = v3d(8, 0, 8);
		points[11] = v3d(2, 0, 8);
		points[12] = v3d(2, -8, 0);
		points[13] = v3d(8, -8, 0);
		points[14] = v3d(10, -8, 2);
		points[15] = v3d(10, -8, 8);
		points[16] = v3d(8, -8, 10);
		points[17] = v3d(2, -8, 10);
		points[18] = v3d(0, -8, 8);
		points[19] = v3d(0, -8, 2);
		points[20] = v3d(2, -10, 2);
		points[21] = v3d(8, -10, 2);
		points[22] = v3d(8, -10, 8);
		points[23] = v3d(2, -10, 8);

		tris[0] = v3d(0, 12, 1);
		tris[1] = v3d(12, 13, 1);
		tris[2] = v3d(1, 13, 14);
		tris[3] = v3d(1, 14, 2);
		tris[4] = v3d(2, 14, 15);
		tris[5] = v3d(2, 15, 3);
		tris[6] = v3d(3, 15, 16);
		tris[7] = v3d(3, 16, 4);
		tris[8] = v3d(4, 16, 17);
		tris[9] = v3d(4, 17, 5);
		tris[10] = v3d(5, 17, 18);
		tris[11] = v3d(5, 18, 6);
		tris[12] = v3d(6, 18, 19);
		tris[13] = v3d(6, 19, 7);
		tris[14] = v3d(7, 19, 12);
		tris[15] = v3d(7, 12, 0);
		tris[16] = v3d(19, 20, 12);
		tris[17] = v3d(13, 21, 14);
		tris[18] = v3d(15, 22, 16);
		tris[19] = v3d(17, 23, 18);
		tris[20] = v3d(12, 20, 21);
		tris[21] = v3d(12, 21, 13);
		tris[22] = v3d(14, 21, 22);
		tris[23] = v3d(14, 22, 15);
		tris[24] = v3d(16, 22, 23);
		tris[25] = v3d(16, 23, 17);
		tris[26] = v3d(18, 23, 20);
		tris[27] = v3d(18, 20, 19);
		tris[28] = v3d(20, 22, 21);
		tris[29] = v3d(22, 20, 23);

		move(v3d(-5, 0, -5));
		scale(0.1);
	}

	void draw(SDL_Surface* surface, camera& Camera, v3d bColor) {
		for (int i = 0; i < trisCount; i++) {
			v3d light(0, 1, 0);
			v3d sun(8, -4, 8);
			renderPrim prim;
			prim.pts[0] = points[(int)tris[i].x];
			prim.pts[1] = points[(int)tris[i].y];
			prim.pts[2] = points[(int)tris[i].z];
			v3d avg = prim.pts[0] + prim.pts[1] + prim.pts[2];
			avg /= 3;
			light = avg - sun;
			light /= light.length();
			v3d normal = getNormal(prim.pts);
			float v = sqrtf((normal.dotProduct(light) + 1) / 2);
			int color = getColor(surface, bColor.x * v, bColor.y * v, bColor.z * v);
			prim.color = color;
			Camera.insert(prim);
		}
	}
};

void displayCam(SDL_Surface* surface, camera* Camera, bool flat) {
	for (int i = 0; i < Camera->count; i++) {
		if (Camera->prims[i].drawable)
			drawTriangle(surface, persp(Camera->prims[i].pts[0], flat),
				persp(Camera->prims[i].pts[1], flat),
				persp(Camera->prims[i].pts[2], flat), Camera->prims[i].color);
	}
}

v3d lerp(v3d a, v3d b, float k) {
	v3d diff = b - a;
	diff *= k;
	return a + diff;
}

void drawSnake(SDL_Surface* surface, camera& Camera, snake& Snake, float time) {
	float SIN = sin(5 * time) * 0.05;
	for (int i = 0; i < Snake.length; i++) {
		float k = (i % 2 == 0 ? SIN : -SIN);
		model cube(24, 30);
		v3d color;
		cube.createSnake();
		if (!(i % 2)) {
			cube.scale(1.1);
			color = C1;
		}
		else {
			cube.scale(0.9);
			color = C2;
		}

		if (i == 0) {
			cube.scale(1.1);
			color = HC;
		}

		if (i == Snake.length - 2) {
			v3d basec = (i + 1) % 2 ? C1 : C2;

			color = lerp(TC, basec, Snake.lastSize);
		}

		if (i == Snake.length - 1) {
			v3d prevc = (i + 1) % 2 ? C1 : C2;

			color = lerp(prevc, TC, Snake.lastSize);

			cube.scale(Snake.lastSize);
		}


		if (Snake.dir[i] == LEFT) cube.move(v3d(0, 0, k));
		if (Snake.dir[i] == RIGHT) cube.move(v3d(0, 0, -k));
		if (Snake.dir[i] == UP) cube.move(v3d(-k, 0, 0));
		if (Snake.dir[i] == DOWN) cube.move(v3d(k, 0, 0));

		cube.move(v3d(Snake.pos[i].x, 0, Snake.pos[i].y));
		cube.draw(surface, Camera, color);
	}
}

void drawLevel(SDL_Surface* surface, camera& Camera, v2d size, bool flat) {
	for (int x = -1; x <= size.x; x++) {
		for (int y = -1; y <= size.y; y++) {
			int color;
			if (x == -1 || x == size.x || y == -1 || y == size.y) {
				color = getColor(surface, 237, 42, 17);
				if ((x + 2) % 2 == (y + 2) % 2) color = getColor(surface, 168, 62, 30);
			}
			else {
				color = getColor(surface, 105, 191, 98);
				if (x % 2 == y % 2) color = getColor(surface, 28, 176, 85);
			}
			v3d pts[] = { v3d(x - 0.5, 0, y - 0.5), v3d(x + 1 - 0.5, 0, y - 0.5), v3d(x - 0.5, 0, y + 1 - 0.5), v3d(x + 1 - 0.5, 0, y + 1 - 0.5) };
			bool drawable = 1;
			for (int i = 0; i < 4; i++) {
				//move
				pts[i] -= Camera.position;
				//rotate
				pts[i] = pts[i].rotXZ(Camera.rotation.x);
				pts[i] = pts[i].rotYZ(Camera.rotation.y);
				bool z1 = pts[i].z <= 0.1;
				if (pts[i].z <= 0.1) drawable = 0;
			}
			if (!drawable) continue;

			drawTriangle(surface, persp(pts[0], flat), persp(pts[1], flat), persp(pts[2], flat), color);
			drawTriangle(surface, persp(pts[1], flat), persp(pts[2], flat), persp(pts[3], flat), color);
		}
	}
}

void drawFood(SDL_Surface* surface, camera& Camera, v2d food, float time) {
	float k = 0.2 * (sin(time * 3) + 1.f) / 2.f;
	model cube(24, 30);
	v3d color(2, 121, 247);
	cube.createSnake();
	cube.scale(0.4 + k);
	cube.move(v3d(food.x, 0, food.y));
	cube.draw(surface, Camera, color);
}

void drawRedDot(SDL_Surface* surface, camera& Camera, v2d dot, float time) {
	float k = 0.2 * (sin(time * 3) + 1.f) / 2.f;
	model cube(24, 30);
	v3d color(214, 11, 4);
	cube.createSnake();
	cube.scale(0.4 + k);
	cube.move(v3d(dot.x, 0, dot.y));
	cube.draw(surface, Camera, color);
}

void drawRedDotTimer(SDL_Surface* surface, float dotTime) {
	float k = dotTime / DOTTIME;
	int length = k * 200;
	int backColor = getColor(surface, 150, 20, 20);
	int frontColor = getColor(surface, 240, 20, 20);
	drawScanline(surface, 220, 420, 55, backColor);
	drawScanline(surface, 219, 421, 56, backColor);
	drawScanline(surface, 218, 422, 57, backColor);
	drawScanline(surface, 217, 423, 58, backColor);
	drawScanline(surface, 218, 422, 59, backColor);
	drawScanline(surface, 219, 421, 60, backColor);
	drawScanline(surface, 220, 420, 61, backColor);

	drawScanline(surface, 222, 220 + length, 56, frontColor);
	drawScanline(surface, 221, 220 + length + 1, 57, frontColor);
	drawScanline(surface, 220, 220 + length + 2, 58, frontColor);
	drawScanline(surface, 221, 220 + length + 1, 59, frontColor);
	drawScanline(surface, 222, 220 + length, 60, frontColor);
}

void drawPortal(SDL_Surface* surface, camera& Camera, v2d dot) {
	model cube(24, 30);
	v3d color(30, 30, 30);
	cube.createSnake();
	cube.scale(0.7);
	cube.move(v3d(dot.x, 0, dot.y));
	cube.draw(surface, Camera, color);
}

class sdlContainer {
public:
	SDL_Event event;
	SDL_Surface* screen;

	SDL_Surface* charset;
	SDL_Surface* frame;
	SDL_Surface* menu;
	SDL_Surface* logo;

	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool init(bool fullscreen) {
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			printf("SDL_Init error: %s\n", SDL_GetError());
			return 0;
		}
		short rc;
		if (fullscreen) rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
		else rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 0;
		};
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Snake 3D");

		screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_ShowCursor(SDL_DISABLE);
		charset = SDL_LoadBMP("./images/cs8x8.bmp");
		if (charset == NULL) {
			printf("SDL_LoadBMP(images/cs8x8.bmp) error: %s\n", SDL_GetError());
			return 0;
		};
		SDL_SetColorKey(charset, true, 0x000000);
		frame = SDL_LoadBMP("./images/frame.bmp");
		if (frame == NULL) {
			printf("SDL_LoadBMP(images/frame.bmp) error: %s\n", SDL_GetError());
			return 0;
		};
		menu = SDL_LoadBMP("./images/menu.bmp");
		if (menu == NULL) {
			printf("SDL_LoadBMP(images/menu.bmp) error: %s\n", SDL_GetError());
			return 0;
		};
		logo = SDL_LoadBMP("./images/logo.bmp");
		if (logo == NULL) {
			printf("SDL_LoadBMP(images/logo.bmp) error: %s\n", SDL_GetError());
			return 0;
		};
		return 1;
	}

	~sdlContainer() {
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
	}
};

short runMenu(sdlContainer& SDL) {
	while (1) {
		DrawSurface(SDL.screen, SDL.menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 4);
		DrawSurface(SDL.screen, SDL.logo, SCREEN_WIDTH / 2, 150, 3.5);
		char text[128];

		sprintf(text, "Start game - 'n'");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 20 / 2, 220, text, SDL.charset, 2.5);

		sprintf(text, "Run autoplayer - 'a'");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 20 / 2, 270, text, SDL.charset, 2.5);

		sprintf(text, "Quit - 'Esc'");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 20 / 2, 320, text, SDL.charset, 2.5);


		SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
		SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL.renderer);

		while (SDL_PollEvent(&SDL.event)) {
			switch (SDL.event.type) {
			case SDL_KEYDOWN:
				if (SDL.event.key.keysym.sym == SDLK_ESCAPE) return -1;
				else if (SDL.event.key.keysym.sym == SDLK_n) return 0;
				else if (SDL.event.key.keysym.sym == SDLK_s) return 1;
				else if (SDL.event.key.keysym.sym == SDLK_a) return 2;
				break;
			case SDL_QUIT:
				return -1;
				break;
			};
		};
	}
}

short handleControlls(sdlContainer& SDL, game& Game, camera& Camera) {
	while (SDL_PollEvent(&SDL.event)) {
		switch (SDL.event.type) {
		case SDL_KEYDOWN:
			if (SDL.event.key.keysym.sym == SDLK_ESCAPE) return -1;
			else if (SDL.event.key.keysym.sym == SDLK_RIGHT) {
				Game.Snake.turn = 1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_LEFT) {
				Game.Snake.turn = -1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_DOWN) {
				Camera.zoom += 0.1;
				if (Camera.zoom > 1.0) Camera.zoom = 1.0;
			}
			else if (SDL.event.key.keysym.sym == SDLK_UP) {
				Camera.zoom -= 0.1;
				if (Camera.zoom < 0.0) Camera.zoom = 0.0;
			}
			else if (SDL.event.key.keysym.sym == SDLK_n) {
				return -2;
			}
			else if (SDL.event.key.keysym.sym == SDLK_s) {
				Game.save();
			}
			else if (SDL.event.key.keysym.sym == SDLK_l) {
				Game.load();
			}
			else if (SDL.event.key.keysym.sym == SDLK_m) {
				Game.flat = !Game.flat;
			}
			break;
		case SDL_QUIT:
			return -1;
			break;
		}
		return 0;
	}
}

short handleFlatControlls(sdlContainer& SDL, game& Game, camera& Camera) {
	while (SDL_PollEvent(&SDL.event)) {
		switch (SDL.event.type) {
		case SDL_KEYDOWN:
			if (SDL.event.key.keysym.sym == SDLK_ESCAPE) return -1;
			else if (SDL.event.key.keysym.sym == SDLK_RIGHT) {
				if (Game.Snake.dir[0] == UP) Game.Snake.turn = 1;
				else if (Game.Snake.dir[0] == DOWN) Game.Snake.turn = -1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_LEFT) {
				if (Game.Snake.dir[0] == UP) Game.Snake.turn = -1;
				else if (Game.Snake.dir[0] == DOWN) Game.Snake.turn = 1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_DOWN) {
				if (Game.Snake.dir[0] == RIGHT) Game.Snake.turn = 1;
				else if (Game.Snake.dir[0] == LEFT) Game.Snake.turn = -1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_UP) {
				if (Game.Snake.dir[0] == RIGHT) Game.Snake.turn = -1;
				else if (Game.Snake.dir[0] == LEFT) Game.Snake.turn = 1;
			}
			else if (SDL.event.key.keysym.sym == SDLK_n) {
				return -2;
			}
			else if (SDL.event.key.keysym.sym == SDLK_s) {
				Game.save();
			}
			else if (SDL.event.key.keysym.sym == SDLK_l) {
				Game.load();
			}
			else if (SDL.event.key.keysym.sym == SDLK_m) {
				Game.flat = !Game.flat;
			}
			break;
		case SDL_QUIT:
			return -1;
			break;
		}
		return 0;
	}
}

short handleAutoplayerControlls(sdlContainer& SDL, game& Game, camera& Camera) {
	while (SDL_PollEvent(&SDL.event)) {
		switch (SDL.event.type) {
		case SDL_KEYDOWN:
			if (SDL.event.key.keysym.sym == SDLK_ESCAPE) return -1;
			else if (SDL.event.key.keysym.sym == SDLK_DOWN) {
				Camera.zoom += 0.1;
				if (Camera.zoom > 1.0) Camera.zoom = 1.0;
			}
			else if (SDL.event.key.keysym.sym == SDLK_UP) {
				Camera.zoom -= 0.1;
				if (Camera.zoom < 0.0) Camera.zoom = 0.0;
			}
			else if (SDL.event.key.keysym.sym == SDLK_s) {
				Game.save();
			}
			else if (SDL.event.key.keysym.sym == SDLK_l) {
				Game.load();
			}
			else if (SDL.event.key.keysym.sym == SDLK_m) {
				Game.flat = !Game.flat;
			}
			break;
		case SDL_QUIT:
			return -1;
			break;
		}
		return 0;
	}
}


short runGame(sdlContainer& SDL) {
	int t1, t2, quit, frames;
	double delta, fpsTimer, fps, speedupTime = 0;

	char text[128];

	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	camera Camera;
	game Game;
	while (!quit) {
		Camera.clear();
		t2 = SDL_GetTicks();

		delta = (t2 - t1) * 0.001;
		t1 = t2;

		Game.time += delta;
		speedupTime += delta;

		SDL_FillRect(SDL.screen, NULL, getColor(SDL.screen, 20, 20, 40));

		//speedup of game
		if (speedupTime >= 30) {
			speedupTime -= 30;
			Game.snakeSpeed = sqrtf(Game.snakeSpeed * Game.Settings.speedUp);
		}

		if (Game.Snake.checkFood(Game.food)) {
			Game.score += Game.Settings.foodPts;
			Game.newFood();
		}
		if (Game.checkDot(delta)) Game.score += Game.Settings.dotPts;

		Game.runSnake(delta);
		Game.runPortals();
		Game.follow(Camera, delta);

		if (Game.Snake.checkCollision()) {
			return Game.score;
		}

		drawLevel(SDL.screen, Camera, Game.boardSize, Game.flat);
		drawFood(SDL.screen, Camera, Game.food, Game.time);
		if (Game.dotTime >= 0) drawRedDot(SDL.screen, Camera, Game.dot, Game.time);
		for (int i = 0; i < Game.portalCount; i++) {
			drawPortal(SDL.screen, Camera, Game.portals[i].pos[0]);
			drawPortal(SDL.screen, Camera, Game.portals[i].pos[1]);
		}
		drawSnake(SDL.screen, Camera, Game.Snake, Game.time);

		Camera.run();
		displayCam(SDL.screen, &Camera, Game.flat);
		fpsTimer += delta;
		if (fpsTimer > 1) {
			fps = frames;
			frames = 0;
			fpsTimer -= 1;
		};

		//draw portal numbers
		for (int i = 0; i < 2 * Game.portalCount; i++) {
			v3d pos(Game.portals[i / 2].pos[i % 2].x, -0.2, Game.portals[i / 2].pos[i % 2].y);
			pos -= Camera.position;
			//rotate
			pos = pos.rotXZ(Camera.rotation.x);
			pos = pos.rotYZ(Camera.rotation.y);
			if (pos.z <= 0.1) continue;
			v2d p2d = persp(pos, Game.flat);
			sprintf(text, "%i", i / 2 + 1);
			DrawString(SDL.screen, p2d.x - strlen(text) * 4, p2d.y - 4, text, SDL.charset);
		}

		DrawSurface(SDL.screen, SDL.frame, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 4);

		sprintf(text, "SNAKE 3D");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 20, text, SDL.charset);

		sprintf(text, "Time: %.1lf FPS: %.0lf Points: %i", Game.time, fps, Game.score);
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 36, text, SDL.charset);

		if (Game.dotTime >= 0) {
			drawRedDotTimer(SDL.screen, Game.dotTime);
			sprintf(text, "Red dot: %.1lf", Game.dotTime);
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 65, text, SDL.charset);
		}

		SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
		SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL.renderer);

		short code = Game.flat ? handleFlatControlls(SDL, Game, Camera) : handleControlls(SDL, Game, Camera);
		if (code != 0) return code;
		frames++;
	}
	return -1;
}

short runAutoplayer(sdlContainer& SDL) {
	int t1, t2, frames;
	double delta, fpsTimer, fps, speedupTime = 0;
	char text[128];
	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	camera Camera;
	game Game;
	autoplayer Autoplayer;
	v2d route;
	short hasPlan = 0;
	bool hasDotPlan = 0;
	bool end = 0;

	while (true) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		if(Game.dotTime < 0) hasDotPlan = 0;

		route = Game.food - Game.Snake.pos[0].round();
		if (Game.dotTime > 0 && hasDotPlan) route = Game.dot - Game.Snake.pos[0].round();

		if (!hasPlan) {
			if (Autoplayer.runSimulation1To(Game.food, Game)) hasPlan = 1;
			else if (Autoplayer.runSimulation2To(Game.food, Game)) hasPlan = 2;
			else end = true;
		}

		if (Game.dotTime > 0 && !hasDotPlan) {
			bool s1 = Autoplayer.runSimulation1To(Game.dot, Game);
			bool s2 = Autoplayer.runSimulation2To(Game.dot, Game);
			if (s1) hasPlan = 1;
			else if (s2) hasPlan = 2;
			if (s1 || s2) {
				route = Game.dot - Game.Snake.pos[0].round();
				hasDotPlan = 1;
			}
		}

		if (hasPlan && !end) Autoplayer.run(Game, route, hasPlan, delta);
		Camera.clear();
		Game.time += delta;
		speedupTime += delta;

		SDL_FillRect(SDL.screen, NULL, getColor(SDL.screen, 50, 20, 20));

		if (speedupTime >= 30) {
			speedupTime -= 30;
			Game.snakeSpeed = sqrtf(Game.snakeSpeed * 3);
		}

		if(!end) Game.runSnake(delta);
		Game.follow(Camera, delta);

		if (Game.Snake.checkFood(Game.food)) {
			hasPlan = 0;
			Game.score += 1;
			Game.newFood();
		}
		if (Game.checkDot(delta)) {
			Game.score += 3;
			hasDotPlan = 0;
		}

		drawLevel(SDL.screen, Camera, Game.boardSize, Game.flat);
		drawFood(SDL.screen, Camera, Game.food, Game.time);
		if (Game.dotTime >= 0) drawRedDot(SDL.screen, Camera, Game.dot, Game.time);
		drawSnake(SDL.screen, Camera, Game.Snake, Game.time);

		Camera.run();
		displayCam(SDL.screen, &Camera, Game.flat);
		fpsTimer += delta;
		if (fpsTimer > 1) {
			fps = frames;
			frames = 0;
			fpsTimer -= 1;
		};


		DrawSurface(SDL.screen, SDL.frame, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 4);

		sprintf(text, "AUTOPLAYER");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 25, text, SDL.charset);

		sprintf(text, "Time: %.1lf FPS: %.0lf Points: %i", Game.time, fps, Game.score);
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 36, text, SDL.charset);

		if (Game.dotTime >= 0) {
			drawRedDotTimer(SDL.screen, Game.dotTime);
			sprintf(text, "Red dot: %.1lf", Game.dotTime);
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 8 / 2, 65, text, SDL.charset);
		}

		if (end) {
			sprintf(text, "No possible route");
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 24 / 2, 240, text, SDL.charset, 3);
		}

		SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
		SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL.renderer);

		frames++;

		if (handleAutoplayerControlls(SDL, Game, Camera) == -1) return 0;
	}
	return 0;
}

short runResults(sdlContainer& SDL, int newScore) {
	FILE* file = fopen("highscore.txt", "r");;
	int scores[3];
	char names[3][16];
	for (int i = 0; i < 3; i++) fscanf(file, "%i %s", &scores[i], names[i]);
	int min = scores[2];
	char text[128];
	char name[16] = "";
	bool nameReady = 0;
	fclose(file);
	if (newScore > min) SDL_StartTextInput();
	while (1) {
		while (SDL_PollEvent(&SDL.event)) {
			switch (SDL.event.type) {
			case SDL_TEXTINPUT:
				if (strlen(name) < 15)
					strcat(name, SDL.event.text.text);
				break;
			case SDL_KEYDOWN:
				if (SDL.event.key.keysym.sym == SDLK_ESCAPE) return -1;
				if ((nameReady || newScore < min) && SDL.event.key.keysym.sym == SDLK_n) return -2;
				else if (SDL.event.key.keysym.sym == SDLK_BACKSPACE && strlen(name) > 0) {
					name[strlen(name) - 1] = '\0';
				}
				else if (SDL.event.key.keysym.sym == 13) {
					nameReady = 1;
					FILE* file = fopen("highscore.txt", "w");
					if (newScore > scores[0]) {
						//1 place
						fprintf(file, "%i %s\n", newScore, name);
						fprintf(file, "%i %s\n", scores[0], names[0]);
						fprintf(file, "%i %s\n", scores[1], names[1]);
					}
					else if (newScore > scores[1]) {
						//2 place
						fprintf(file, "%i %s\n", scores[0], names[0]);
						fprintf(file, "%i %s\n", newScore, name);
						fprintf(file, "%i %s\n", scores[1], names[1]);
					}
					else {
						//3 place
						fprintf(file, "%i %s\n", scores[0], names[0]);
						fprintf(file, "%i %s\n", scores[1], names[1]);
						fprintf(file, "%i %s\n", newScore, name);
					}
					fclose(file);
				}
				break;
			case SDL_QUIT:
				return -1;
				break;
			}
		};

		DrawSurface(SDL.screen, SDL.menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 4);

		sprintf(text, "Best scores: ");
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 140, text, SDL.charset, 2);

		for (int i = 0; i < 3; i++) {
			sprintf(text, "%i. %s - %i pts", i + 1, names[i], scores[i]);
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 160 + 20 * i, text, SDL.charset, 2);
		}

		sprintf(text, "You scored %i points", newScore);
		DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 220, text, SDL.charset, 2);

		if (newScore > min) {
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 240, "New best score!", SDL.charset, 2);
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 260, "Enter your name: ", SDL.charset, 2);

			if (!nameReady) {
				sprintf(text, "%s", name);
				DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 280, text, SDL.charset, 2);
			}
			else DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 280, "Score saved!", SDL.charset, 2);
		}
		else {
			sprintf(text, "Try again!", newScore);
			DrawString(SDL.screen, SDL.screen->w / 2 - strlen(text) * 16 / 2, 240, text, SDL.charset, 2);
		}

		DrawString(SDL.screen, SDL.screen->w / 2 - 9 * 16, 300, "Exit - press 'esc'", SDL.charset, 2);
		DrawString(SDL.screen, SDL.screen->w / 2 - 10 * 16, 320, "New game - press 'n'", SDL.charset, 2);

		SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
		SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL.renderer);
	}

	SDL_StopTextInput();
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	sdlContainer SDL;
	if (!SDL.init(true)) return 0;

	srand(time(NULL));

	while (1) {
		int mode = runMenu(SDL);
		int score = -2, res = 0;
		switch (mode) {
		case -1:
			SDL_Quit;
			return 0;
		case 0:
			while (score == -2) {
				score = runGame(SDL);
				if (score == -1) return 0;
				else if (score >= 0) res = runResults(SDL, score);

				if (res == -1) return 0;
				else if (res == -2) score = -2;
			}
			break;
		case 2:
			if(runAutoplayer(SDL) == 0) return 0;
			break;
		}

	}

	SDL_Quit();
	return 0;
};
