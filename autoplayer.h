#pragma once
#include "game.h"
#include "utility.h"
class autoplayer {
public:
	bool runSimulation1To(v2d des, game& Game) {
		int length = Game.Snake.length;
		v2d head = Game.Snake.pos[0].round();
		int headDir = Game.Snake.dir[0];
		v2d* body = (v2d*)malloc((length - 1) * sizeof(v2d));
		for (int i = 0; i < length - 1; i++) {
			body[i] = Game.Snake.pos[i + 1].round();
		}
		v2d dist = des - head;
		v2d sign(dist.x >= 0 ? 1 : -1, dist.y >= 0 ? 1 : -1);
		int steps = fabs(dist.x) + fabs(dist.y);
		if (sign.x == -1 && headDir == RIGHT || sign.x == 1 && headDir == LEFT) return 0;

		for (int i = 0; i < steps; i++) {
			for (int j = length - 2; j > 0; j--) {
				body[j] = body[j - 1];
			}

			body[0] = head;
			if (dist.x > 0) {
				head.x += 1;
				dist.x -= 1;
			}
			else if(dist.x < 0) {
				head.x -= 1;
				dist.x += 1;
			}
			else if (dist.y > 0) {
				head.y += 1;
				dist.y -= 1;
			}
			else if (dist.y < 0) {
				head.y -= 1;
				dist.y += 1;
			}
			for (int j = 0; j < length - 1; j++) {
				if (head == body[j]) return 0;
			}
		}
		return 1;
		free(body);
	}
	bool runSimulation2To(v2d des, game& Game) {
		int length = Game.Snake.length;
		v2d head = Game.Snake.pos[0].round();
		int headDir = Game.Snake.dir[0];
		v2d* body = (v2d*)malloc((length - 1) * sizeof(v2d));
		for (int i = 0; i < length - 1; i++) {
			body[i] = Game.Snake.pos[i + 1].round();
		}
		v2d dist = des - head;
		v2d sign(dist.x >= 0 ? 1 : -1, dist.y >= 0 ? 1 : -1);
		int steps = fabs(dist.x) + fabs(dist.y);
		if (sign.y == -1 && headDir == UP || sign.y == 1 && headDir == DOWN) return 0;

		for (int i = 0; i < steps; i++) {
			for (int j = length - 2; j > 0; j--) {
				body[j] = body[j - 1];
			}
			body[0] = head;
			if (dist.y > 0) {
				head.y += 1;
				dist.y -= 1;
			}
			else if (dist.y < 0) {
				head.y -= 1;
				dist.y += 1;
			}
			else if (dist.x > 0) {
				head.x += 1;
				dist.x -= 1;
			}
			else if (dist.x < 0) {
				head.x -= 1;
				dist.x += 1;
			}
			for (int j = 0; j < length - 1; j++) {
				if (head == body[j]) return 0;
			}
		}
		return 1;
		free(body);
	}

	void run(game &Game, v2d &route, short mode, float delta) {
		float step = delta * Game.snakeSpeed;
		float dir = -1;
		if (mode == 1) {
			if (route.x < 0) dir = LEFT;
			else if (route.x > 0) dir = RIGHT;
			else if (route.y > 0) dir = UP;
			else if (route.y < 0) dir = DOWN;
		}
		else if (mode == 2) {
			if (route.y > 0) dir = UP;
			else if (route.y < 0) dir = DOWN;
			else if (route.x < 0) dir = LEFT;
			else if (route.x > 0) dir = RIGHT;
		}
		int offset = dir - Game.Snake.dir[0];
		if (offset < 0) offset += 4;

		switch (offset) {
		case 3: 
			Game.Snake.turn = -1;
			break;
		case 0:
			Game.Snake.turn = 0;
			break;
		case 1: 
			Game.Snake.turn = 1;
			break;
		}
		
	}
};