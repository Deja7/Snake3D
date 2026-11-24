#pragma once
#include <math.h>
#include <time.h>
#include <stdlib.h>
class v2d {
public:
	float x;
	float y;
	v2d() { x = 0; y = 0; }
	v2d(float a, float b) { x = a; y = b; }
	v2d operator+(v2d sec) { return v2d(x + sec.x, y + sec.y); }
	v2d operator-(v2d sec) { return v2d(x - sec.x, y - sec.y); }
	void operator*=(float k) { x *= k; y *= k; }
	bool operator==(v2d sec) { return x == sec.x && y == sec.y; }
	void operator/=(float k) { x /= k; y /= k; }
	v2d round() { return v2d(roundf(x), roundf(y)); }
	void operator+=(v2d sec) {
		x += sec.x;
		y += sec.y;
	}
	float length() { return sqrtf(x * x + y * y); }
};

class v3d {
public:
	float x;
	float y;
	float z;
	v3d() { x = 0; y = 0; z = 0; }
	v3d(float a, float b, float c) { x = a; y = b; z = c; }
	v3d operator+(v3d sec) { return v3d(x + sec.x, y + sec.y, z + sec.z); }
	v3d operator-(v3d sec) { return v3d(x - sec.x, y - sec.y, z - sec.z); }
	void operator+=(v3d sec) { x += sec.x; y += sec.y; z += sec.z; }
	void operator-=(v3d sec) { x -= sec.x; y -= sec.y; z -= sec.z; }
	void operator*=(float k) { x *= k; y *= k; z *= k; }
	void operator/=(float k) { x /= k; y /= k; z /= k; }
	float length() { return sqrtf(x * x + y * y + z * z); }
	float dotProduct(v3d sec) {return x * sec.x + y * sec.y + z * sec.z;}

	v3d rotXZ(float deg) {
		v3d t; t.x = x; t.y = y; t.z = z;
		float rad = deg * 3.14 * 2;
		t.z = z * cos(rad) + x * sin(rad);
		t.x = x * cos(rad) - z * sin(rad);
		return t;
	}
	v3d rotXY(float deg) {
		v3d t; t.x = x; t.y = y; t.z = z;
		float rad = deg * 3.14 * 2;
		t.y = y * cos(rad) + x * sin(rad);
		t.x = x * cos(rad) - y * sin(rad);
		return t;
	}
	v3d rotYZ(float deg) {
		v3d t; t.x = x; t.y = y; t.z = z;
		float rad = deg * 3.14 * 2;
		t.z = z * cos(rad) + y * sin(rad);
		t.y = y * cos(rad) - z * sin(rad);
		return t;
	}
};

v2d persp(v3d v, bool flat) {
	v2d SCREEN(640, 480);
	if(flat) return v2d(v.x * 16 + SCREEN.x / 2, v.y * 16 + SCREEN.y / 2);
	v2d out;	 
	out.x = (v.x) / (v.z * 4) * SCREEN.x + SCREEN.x / 2;
	out.y = (v.y) / (v.z * 3) * SCREEN.y + SCREEN.y / 2;
	return out;
}

class renderPrim {
public:
	bool drawable = 1;
	v3d pts[3];
	int color;
	float dist;
};

float sq(float a) {
	return a * a;
}
int sort(const renderPrim *r1, const renderPrim *r2) {
	if (r1->dist < r2->dist) return 1;
	if (r1->dist > r2->dist) return -1;
	return 0;
}

class camera {
public:
	renderPrim prims[10000];
	int count = 0;
	v2d rotation = v2d(0, 0.25);
	v3d position = v3d(0, -3, -2);
	float zoom = 0;
	void clear() {
		count = 0;
	}
	void insert(renderPrim prim) {
		prims[count] = prim;
		count++;
	}
	void run() {
		//sort based on distance
		for (int i = 0; i < count; i++) {
			v3d tmp = prims[i].pts[0] + prims[i].pts[1] + prims[i].pts[2];
			tmp /= 3;
			prims[i].dist = sqrtf(sq(tmp.x - position.x) + sq(tmp.y - position.y) + sq(tmp.z - position.z));
		}

		qsort(prims, count, sizeof(*prims), (int(*)(const void*, const void*))sort);

		//move relative to camera
		for (int i = 0; i < count; i++) {
			prims[i].pts[0] -= position;
			prims[i].pts[1] -= position;
			prims[i].pts[2] -= position;
		}

		//rotate relative to camera
		for (int i = 0; i < count; i++) {
			//rotate horizontally
			prims[i].pts[0] = prims[i].pts[0].rotXZ(rotation.x);
			prims[i].pts[1] = prims[i].pts[1].rotXZ(rotation.x);
			prims[i].pts[2] = prims[i].pts[2].rotXZ(rotation.x);

			//rotate vertically
			prims[i].pts[0] = prims[i].pts[0].rotYZ(rotation.y);
			prims[i].pts[1] = prims[i].pts[1].rotYZ(rotation.y);
			prims[i].pts[2] = prims[i].pts[2].rotYZ(rotation.y);
		}

		//filter unwanted ones
		for (int i = 0; i < count; i++) {
			bool z1 = prims[i].pts[0].z <= 0.1;
			bool z2 = prims[i].pts[1].z <= 0.1;
			bool z3 = prims[i].pts[2].z <= 0.1;
			if (z1 || z2 || z3) prims[i].drawable = 0;
		}

	}
};

v3d getNormal(v3d p[3]) {
	v3d normal;
	v3d A = p[1] - p[0];
	v3d B = p[2] - p[0];
	normal.x = A.y * B.z - A.z * B.y;
	normal.y = A.z * B.x - A.x * B.z;
	normal.z = A.x * B.y - A.y * B.x;
	float normalL = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= normalL; normal.y /= normalL; normal.z /= normalL;
	return normal;
}

class portal {
public:
	v2d pos[2];
	int num;
};

class settings {
public:
	float speed;
	int length;
	int frequency;
	int foodPts;
	int dotPts;
	int boardX;
	int boardY;
	float speedUp;
	float speedDown;

	void load() {
		FILE* file;
		file = fopen("settings.txt", "r");
		fscanf(file, "%f speed", &speed);
		fscanf(file, "%i length", &length);
		fscanf(file, "%i frequency", &frequency);
		fscanf(file, "%i foodPts", &foodPts);
		fscanf(file, "%i dotPts", &dotPts);
		fscanf(file, "%i boardX", &boardX);
		fscanf(file, "%i boardY", &boardY);
		fscanf(file, "%f speedUp", &speedUp);
		fscanf(file, "%f speedDown", &speedDown);
		fclose(file);
	}
};