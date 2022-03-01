#include "world.h"

using namespace siv;

/*
* using perlin noise generates terrain with higher and lower regions
*/
void World::generateTerrain(vector<vector<float>>* terrain) {

	PerlinNoise perlin(1234);

	for (size_t y = 0; y < HEIGHT; ++y)
	{
		for (size_t x = 0; x < WIDTH; ++x)
		{
			float value = (float)perlin.accumulatedOctaveNoise2D_0_1(x/fx, y/fy, octaves);
			(*terrain)[y][x] = value;
		}
	}
}

/*
* generating textures from given pictures
*/
void World::generateTextures() {

	for (size_t i = 0; i < paths.size(); i++)
	{
		SDL_Surface* picture = SDL_LoadBMP(paths[i].c_str());
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, picture);
		textures.push_back(move(texture));
	}
}

/*
* generating beings at the beginning of the simulation
*/
void World::generateDefaultBeings() {

	const float LOWLAND = 0.6f;
	const float HIGHLAND = 0.3f;
	const float LOW_PROB = SIZE * 0.00001f;
	const float MID_PROB = SIZE * 0.00005f;
	const float HIGH_PROB = SIZE * 0.0002f;

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			int value = rand() % SIZE;


			if (value < LOW_PROB && terrain[y][x] > 0.3f && terrain[y][x] < LOWLAND) {
				nature.push_back(make_unique<RedFlower>(Point(x, y), this));
				beings[y][x] = ListBeings::RED_FLOWER;
			}
			else if (value < MID_PROB && terrain[y][x] > HIGHLAND) {
				animals.push_back(make_unique<Sheep>(Point(x, y), this));
				beings[y][x] = ListBeings::SHEEP;
			}
			else if (value < HIGH_PROB && terrain[y][x] > LOWLAND) {
				nature.push_back(make_unique<VioletFlower>(Point(x, y), this));
				beings[y][x] = ListBeings::VIOLET_FLOWER;
			}
			else
				beings[y][x] = ListBeings::EMPTY;

		}
	}
}

/*
* applies to perlin-noise-generated terrain colors and writes it to the target bitmap
*/
void World::RenderTerrain(unsigned char* target) {

	for (size_t y = 0; y < HEIGHT; ++y)
	{
		for (size_t x = 0; x < WIDTH; ++x)
		{

			auto floor_value = (floor)(terrain[y][x] * 10.) / 10.;
			vector<int> colors = Converter::hsvToRgb(HUE / FULL_CIRCLE, 1, floor_value);
			size_t current_pixel = static_cast<size_t>(3) * (y * WIDTH + x);
			target[current_pixel + static_cast<uint8_t>(Color::R)] = colors[static_cast<uint8_t>(Color::R)];
			target[current_pixel + static_cast<uint8_t>(Color::G)] = colors[static_cast<uint8_t>(Color::G)];
			target[current_pixel + static_cast<uint8_t>(Color::B)] = colors[static_cast<uint8_t>(Color::B)];
		}
	}
}

void World::RenderBeings() {

	renderArray(&nature);
	renderArray(&animals);
}

/*
* from the given array takes the correct textures and "glues" it to the result bitmap
*/
void World::renderArray(vector<being_ptr>* arr){
	SDL_Rect rect;
	for (size_t i = 0; i < arr->size(); ++i)
	{
		rect.w = (*arr)[i]->GetSize().width;
		rect.h = (*arr)[i]->GetSize().height;
		rect.x = (*arr)[i]->Position.x - (rect.w >> 1);
		rect.y = (*arr)[i]->Position.y - (rect.h >> 1);

		auto ID = static_cast<uint8_t>((*arr)[i]->GetBeing());
		SDL_RenderCopy(renderer, textures[ID], NULL, &rect);
	}
}

/*
* for every being calls its simulate method
*/
void World::Simulate() {
	for (size_t i = 0; i < nature.size(); ++i)
		nature[i]->Simulate();

	for (size_t i = 0; i < animals.size(); ++i)
		animals[i]->Simulate();
}


Point World::GetResultPosition(Being* being, const Point& direction) {
	return (being->Position + direction) % WorldSize;
}

