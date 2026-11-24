#pragma once
#include "utility.h"
#include <stdio.h>
#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define DOTTIME 10
#define BLOCKSIZE 16
v2d dirToV2d(int dir) {
	switch (dir) {
	case LEFT:
		return v2d(-1, 0);
	case UP:
		return v2d(0, 1);
	case RIGHT:
		return v2d(1, 0);
	case DOWN:
		return v2d(0, -1);
	default:
		return v2d();
	}
}

class snake {
private:
	short turnLim(short t) {
		if (t < 0) return t + 4;
		else if (t >= 4) return t - 4;
		return t;
	}
	void moveBody(float speed){
		for (int i = 0; i < length - (newPiece == 0 ? 0 : 1); i++) {
			if (dir[i] == LEFT) pos[i].x -= speed;
			else if (dir[i] == RIGHT) pos[i].x += speed;
			else if (dir[i] == UP) pos[i].y += speed;
			else if (dir[i] == DOWN) pos[i].y -= speed;
		}
	}
	void roundBody() {
		for (int i = 0; i < length; i++) {
			if (portalBan[i] > 0) portalBan[i]--;
			pos[i].x = roundf(pos[i].x);
			pos[i].y = roundf(pos[i].y);
		}
		if(newPiece > 0) newPiece--;
	}

public:
	int length;
	short* dir;
	v2d* pos;
	short* portalBan;
	short turn = 0;
	short newPiece = 0;
	float lastSize = 1;
	int blocks = 0;

	void resize(bool init) {
		int level = length / BLOCKSIZE + 1;
		if (level <= blocks) return;
		blocks = level;

		short* dirT, * portalBanT;
		v2d* posT;

		if (!init) {
			dirT = dir;
			portalBanT = portalBan;
			posT = pos;
		}

		dir = (short*)malloc(level * BLOCKSIZE * sizeof(short));
		pos = (v2d*)malloc(level * BLOCKSIZE * sizeof(v2d));
		portalBan = (short*)malloc(level * BLOCKSIZE * sizeof(short));

		if (!init) {
			for (int i = 0; i < length; i++) {
				dir[i] = dirT[i];
				pos[i] = posT[i];
				portalBan[i] = portalBanT[i];
			}
			free(dirT);
			free(portalBanT);
			free(posT);
		}
	}


	snake() {
		resize(true);
		for (int i = 0; i < length; i++) {
			pos[i] = v2d(0, length-i);
			dir[i] = UP;
			portalBan[i] = 0;
		}
	}
	~snake() {
		free(dir);
		free(pos);
		free(portalBan);
	}
	void create(int LENGTH) {
		length = LENGTH;
		resize(true);
		for (int i = 0; i < length; i++) {
			pos[i] = v2d(0, length - i - 1);
			dir[i] = UP;
			portalBan[i] = 0;
		}
	}
	void updateDir(v2d board) {
		for (int i = length - 1; i > 0; i--) {
			dir[i] = dir[i - 1];
		}
		bool isValid[3];
		for (int i = 0; i < 3; i++) {
			v2d nextPos = pos[0] + dirToV2d(turnLim(dir[0] - 1 + i));
			isValid[i] = !(nextPos.x == -1 || nextPos.x == board.x || nextPos.y == -1 || nextPos.y == board.y);
		}
		if (turn == 0 && !isValid[1]) {
			if (isValid[2]) dir[0] = turnLim(dir[0] + 1);
			else dir[0] = turnLim(dir[0] - 1);
		}
		else if (turn == 1) {
			if (isValid[2]) dir[0] = turnLim(dir[0] + 1);
			else if (!isValid[1]) dir[0] = turnLim(dir[0] - 1);
		}
		else if (turn == -1) {
			if (isValid[0]) dir[0] = turnLim(dir[0] - 1);
			else if (!isValid[1]) dir[0] = turnLim(dir[0] + 1);
		}

		turn = 0;
	}

	void move(float speed, v2d board) {
		if (dir[0] == LEFT) {
			if (ceil(pos[0].x - speed) == ceil(pos[0].x)) moveBody(speed);
			else {
				moveBody(speed);
				roundBody();
				updateDir(board);
			}
		}
		else if (dir[0] == RIGHT) {
			if (floor(pos[0].x + speed) == floor(pos[0].x)) moveBody(speed);
			else {
				moveBody(speed);
				roundBody();
				updateDir(board);
			}
		}
		else if (dir[0] == UP) {
			if (floor(pos[0].y + speed) == floor(pos[0].y)) moveBody(speed);
			else {
				moveBody(speed);
				roundBody();
				updateDir(board);
			}
		}
		else if (dir[0] == DOWN) {
			if (ceil(pos[0].y - speed) == ceil(pos[0].y)) moveBody(speed);
			else {
				moveBody(speed);
				roundBody();
				updateDir(board);
			}
		}
	}
	bool checkCollision() {
		if (length == 2) return 0;
		v2d head = pos[0];
		for (int i = 1; i < length; i++) {
			float dist = sqrtf(sq(pos[i].x - head.x) + sq(pos[i].y - head.y));
			if (dist <= 0.5) return 1;
		}
		return 0;
	}
	bool checkFood(v2d food) {
		float dist = sqrtf(sq(pos[0].x - food.x) + sq(pos[0].y - food.y));
		if (dist <= 0.2) {
			length++;
			resize(false);
			portalBan[length - 1] = 0;
			pos[length - 1] = pos[length - 2];
			dir[length - 1] = -1;
			newPiece = 2;
			lastSize = 0;
			return 1;
		}
		return 0;
	}
	bool checkDot(v2d dot) {
		float dist = sqrtf(sq(pos[0].x - dot.x) + sq(pos[0].y - dot.y));
		return dist <= 0.2;
	}
};

class game {
private:
	bool isSpaceFree(v2d pos) {
		for (int i = 0; i < Snake.length; i++) {
			if (roundf(Snake.pos[i].x) == pos.x && roundf(Snake.pos[i].y) == pos.y) return 0;
		}
		if (pos == food) return 0;
		if (pos == dot) return 0;
		for (int i = 0; i < portalCount; i++) {
			if (pos == portals[i].pos[0] || pos == portals[i].pos[1]) return 0;
		}
		return 1;
	}
	v2d getFreeSpace() {
		bool fine = 0;
		v2d tmp;
		while (!fine) {
			tmp = v2d(rand() % (int)(boardSize.x - 2) + 1, rand() % (int)(boardSize.y - 2) + 1);
			fine = isSpaceFree(tmp);
		}
		return tmp;
	}
public:
	int portalCount;
	int score;
	float time;
	v2d food;
	v2d dot;
	float dotTime;
	float newDotIn;
	v2d boardSize;
	snake Snake;
	float snakeSpeed;
	portal *portals;
	bool flat = 0;
	settings Settings;
	game() {
		Settings.load();
		Snake.create(Settings.length);
		snakeSpeed = Settings.speed;
		portalCount = rand()%3;
		score = 0;
		time = 0;
		newDotIn = (float)Settings.frequency;
		boardSize = v2d(Settings.boardX, Settings.boardY);
		portals = (portal*)malloc(portalCount * sizeof(portal));
		for (int i = 0; i < portalCount; i++) {
			bool fine = 0;
			v2d tmp;
			while (!fine) {
				tmp = v2d(rand() % (int)boardSize.x, rand() % (int)boardSize.y);
				fine = isSpaceFree(tmp);
			}
			dot = tmp;
			portals[i].pos[0] = getFreeSpace();
			portals[i].pos[1] = getFreeSpace();
		}
		newFood();
		newDot();
	}
	~game(){
		free(portals);
	}

	void runPortals() {
		for (int s = 0; s < Snake.length; s++) {
			if (Snake.portalBan[s]) continue;
			for (int i = 0; i < portalCount; i++) {
				if ((Snake.pos[s] - portals[i].pos[0]).length() < 0.25) {
					Snake.portalBan[s] = 2;
					Snake.pos[s] += portals[i].pos[1] - portals[i].pos[0];
				}
				else if ((Snake.pos[s] - portals[i].pos[1]).length() < 0.25) {
					Snake.portalBan[s] = 2;
					Snake.pos[s] += portals[i].pos[0] - portals[i].pos[1];
				}
			}
		}
	}

	void runSnake(float delta) {
		float speed = delta * snakeSpeed;
		Snake.move(speed, boardSize);
		Snake.lastSize += delta;
		if (Snake.lastSize > 1) Snake.lastSize = 1;
	}

	void follow(camera& Camera, float delta) {
		if (flat) {
			Camera.position = v3d(boardSize.x / 2, -(boardSize.x + boardSize.y) / 4, boardSize.y / 2);
			Camera.rotation.y = 0.25;
			Camera.rotation.x = 0;
		}
		else {
			//rotation
			float desRot = 0;
			if (Snake.dir[0] == UP) desRot = 0;
			if (Snake.dir[0] == RIGHT) desRot = 0.25;
			if (Snake.dir[0] == DOWN) desRot = 0.5;
			if (Snake.dir[0] == LEFT) desRot = 0.75;

			desRot -= Camera.rotation.x;
			if (desRot < -0.5) desRot += 1.f;
			else if (desRot > 0.5) desRot -= 1.f;

			float rSpeed = 0.35;

			if (desRot > 0.001) Camera.rotation.x += rSpeed * delta < desRot ? rSpeed * delta : desRot;
			else if (desRot < -0.001) Camera.rotation.x += rSpeed * delta < fabs(desRot) ? -rSpeed * delta : desRot;
			else if(desRot > 0) Camera.rotation.x += desRot;
			else if(desRot < 0) Camera.rotation.x += desRot;

			while (Camera.rotation.x >= 1.f) Camera.rotation.x -= 1.f;
			while (Camera.rotation.x < 0.f) Camera.rotation.x += 1.f;
			//position
			v3d desPos(0, -3 - 5 * Camera.zoom, -0.8);
			desPos = desPos.rotXZ(-Camera.rotation.x);
			Camera.position = v3d(Snake.pos[0].x, 0, Snake.pos[0].y) + desPos;
			Camera.rotation.y = 0.1 + 0.15 * Camera.zoom;
		}
	}
	bool checkDot(float delta) {
		dotTime -= delta;
		if (dotTime < 0 && fabs(dotTime) > newDotIn) {
			newDot();
			dotTime = DOTTIME;
		}

		if (dotTime >= 0 && Snake.checkDot(dot)) {
			if (rand() % 2 && Snake.length > 5) {
				Snake.length -= 3;
			}
			else snakeSpeed /= Settings.speedDown;
			dot = v2d(-1, -1);
			dotTime = -1;
			return true;
		}
		return false;
	}
	void newDot() {
		newDotIn = Settings.frequency + rand() % 11;
		dot = getFreeSpace();
	}
	void newFood() {
		food = getFreeSpace();
	}
	void save() {
		FILE* file; 
		file = fopen("save.txt", "w");
		fprintf(file, "%i snakeLength\n", Snake.length);
		for (int i = 0; i < Snake.length; i++) {
			fprintf(file, "%f %f %i %i\n", Snake.pos[i].x, Snake.pos[i].y, Snake.dir[i], Snake.portalBan[i]);
		}
		fprintf(file, "%f lastSize\n", Snake.lastSize);
		fprintf(file, "%i newPiece\n", Snake.newPiece);
		fprintf(file, "%i score\n", score);
		fprintf(file, "%f time\n", time);
		fprintf(file, "%f dotTime\n", dotTime);
		fprintf(file, "%f snakeSpeed\n", snakeSpeed);
		fprintf(file, "%f %f food\n", food.x, food.y);
		fprintf(file, "%f %f dot\n", dot.x, dot.y);
		fprintf(file, "%f %f boardSize\n", boardSize.x, boardSize.y);
		fprintf(file, "%i portalCount\n", portalCount);
		for (int i = 0; i < portalCount; i++) {
			fprintf(file, "%f %f %f %f\n", portals[i].pos[0].x, portals[i].pos[0].y, portals[i].pos[1].x, portals[i].pos[1].y);
		}
		fclose(file);
	}
	void load() {
		FILE* file;
		file = fopen("save.txt", "r");
		if (!file) return;
		//char string[128];
		fscanf(file, "%i snakeLength", &Snake.length);
		Snake.resize(true);
		for (int i = 0; i < Snake.length; i++) {
			fscanf(file, "%f %f %i %i", &Snake.pos[i].x, &Snake.pos[i].y, &Snake.dir[i], &Snake.portalBan[i]);
		}
		fscanf(file, "%f lastSize", &Snake.lastSize);
		fscanf(file, "%i newPiece", &Snake.newPiece);
		fscanf(file, "%i score", &score);
		fscanf(file, "%f time", &time);
		fscanf(file, "%f dotTime", &dotTime);
		fscanf(file, "%f snakeSpeed", &snakeSpeed);
		fscanf(file, "%f %f food", &food.x, &food.y);
		fscanf(file, "%f %f dot", &dot.x, &dot.y);
		fscanf(file, "%f %f boardSize", &boardSize.x, &boardSize.y);
		fscanf(file, "%i portalCount", &portalCount);
		free(portals);
		portals = (portal*)malloc(portalCount * sizeof(portal));
		for (int i = 0; i < portalCount; i++) {
			fscanf(file, "%f %f %f %f", &portals[i].pos[0].x, &portals[i].pos[0].y, &portals[i].pos[1].x, &portals[i].pos[1].y);
		}
		fclose(file);
	}
};
