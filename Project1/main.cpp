//  based on tutorial http://lazyfoo.net/tutorials/SDL/
//
//  background image source https://i.redd.it/g9rfbj50xchy.png

#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>
#include <string>
#include <ctime>
#include <list>

const int screen_width = 640;
const int screen_height = 480;
int gameStatus;
int difficultyLevel;

using namespace::std;

class l_texture
{
public:
	//Initializes variables
	l_texture();

	//Deallocates memory
	~l_texture();

	//Loads image at specified path
	bool loadFromFile(const string& path);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue) const;

	//Set blending
	void setBlendMode(SDL_BlendMode blending) const;

	//Set alpha modulation
	void setAlpha(Uint8 alpha) const;

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

	//Gets image dimensions
	int getWidth() const;
	int getHeight() const;

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

class Difficulty
{
public:
	int shipMovement;
	int bombMovement;
	int submarineMovement;
	int barrelFrequency;
	int barrelMovement;
	Difficulty();
};

class Menu
{
public:
	Menu();
	void handleEvent(SDL_Event & e, int& difficultyLevel, Difficulty& difficulty);
	static void render();
};

class Background
{
public:
	Background();
	static void render();
};

class WinScreen
{
public:
	WinScreen();
	static void render();
};

class LoseScreen
{
public:
	LoseScreen();
	static void render();
};

class Ship
{
public:
	static const int SHIP_WIDTH = 40;
	static const int SHIP_HEIGHT = 21;
	static const int SHIP_VEL = 1;
	Ship();
	int mPosX, mPosY;

	void handleEvent(SDL_Event & e);
	void move(Difficulty& difficulty);
	void render() const;
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;

};

class Bomb
{
public:
	static const int BOMB_WIDTH = 8;
	static const int BOMB_HEIGHT = 8;
	static const int BOMB_VEL = 2;

	Bomb();
	int mPosX, mPosY;

	void handleEvent(SDL_Event& e);
	void move(Ship & ship, Difficulty& difficulty);
	void render() const;

	bool activated = false;
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;
};

class Submarine
{
public:
	static const int SUBMARINE_WIDTH = 27;
	static const int SUBMARINE_HEIGHT = 20;
	static const int SUBMARINE_VEL = 2;

	Submarine();

	void move(Bomb & bomb, Difficulty& difficulty, int& gameStatus);
	void render() const;

	bool facingRight = true;
	int mPosX, mPosY;

private:
	int mVelX, mVelY;
	SDL_Rect mCollider{};
};

class Barrel
{
public:
	static const int BARREL_WIDTH = 10;
	static const int BARREL_HEIGHT = 8;
	static const int BARREL_VEL = 2;

	Barrel();
	int acceleration;
	int velFromAcceleration;
	int mPosX, mPosY;
	int mVelX, mVelY;
	bool available = true;
	bool activated = false;

	void move(Submarine& submarine, Ship& ship, SDL_Rect& waterSurface, int id, int& gameStatus, Difficulty& difficulty);
	void render() const;

	SDL_Rect mCollider{};
};

bool init();
bool loadMedia();
void close();
bool checkCollision(SDL_Rect a, SDL_Rect b);

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;


l_texture gBackgroundSeaTexture;
l_texture gShipTexture;
l_texture gSubmarineTexture;
l_texture gBombTexture;
l_texture gBarrelTexture;
l_texture gWinScreenTexture;
l_texture gLoseScreenTexture;
l_texture gMenuTexture;

l_texture::l_texture()
{
	mTexture = nullptr;
	mWidth = 0;
	mHeight = 0;
}

l_texture::~l_texture()
{
	free();
}

bool l_texture::loadFromFile(const string& path)
{
	free();

	SDL_Texture* newTexture = nullptr;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == nullptr)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 0, 255));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == nullptr)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != nullptr;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}


	//Return success
	return mTexture != NULL;
}
#endif

void l_texture::free()
{
	if (mTexture != nullptr)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
		mWidth = 0;
		mHeight = 0;
	}
}

void l_texture::setColor(const Uint8 red, Uint8 green, Uint8 blue) const
{
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void l_texture::setBlendMode(SDL_BlendMode blending) const
{
	SDL_SetTextureBlendMode(mTexture, blending);
}

void l_texture::setAlpha(Uint8 alpha) const
{
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void l_texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) const
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != nullptr)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int l_texture::getWidth() const
{
	return mWidth;
}

int l_texture::getHeight() const
{
	return mHeight;
}

Background::Background()
= default;

WinScreen::WinScreen()
= default;

LoseScreen::LoseScreen()
= default;

Menu::Menu()
= default;

Difficulty::Difficulty()
{
	shipMovement = 1;
	bombMovement = 1;
	submarineMovement = 1;
	barrelFrequency = 1;
	barrelMovement = 1;
}


Ship::Ship()
{
	mPosX = 0;
	mPosY = 80;

	mCollider.w = SHIP_WIDTH;
	mCollider.h = SHIP_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

Bomb::Bomb()
{
	mPosX = 666;
	mPosY = 666;

	mCollider.w = BOMB_WIDTH;
	mCollider.h = BOMB_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

Submarine::Submarine()
{
	mPosX = 0;
	mPosY = 200 + (rand() % 200);

	mCollider.w = SUBMARINE_WIDTH;
	mCollider.h = SUBMARINE_HEIGHT;

	mVelX = 1;
	mVelY = 0;
}

Barrel::Barrel()
{
	mPosX = 800;
	mPosY = 800;

	acceleration = 0;
	velFromAcceleration = 0;

	mCollider.w = BARREL_WIDTH;
	mCollider.h = BARREL_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

void Ship::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN: mVelY -= SHIP_VEL; break;
		case SDLK_LEFT: mVelX -= SHIP_VEL; break;
		case SDLK_RIGHT: mVelX += SHIP_VEL; break;
		default:
			break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN: mVelY -= SHIP_VEL; break;
		case SDLK_LEFT: mVelX += SHIP_VEL; break;
		case SDLK_RIGHT: mVelX -= SHIP_VEL; break;
		default:
			break;
		}
	}
}

void Bomb::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN:
			if (mPosY < 20) {
				mVelY = 1;
				activated = true;
			}
			break;
		default:
			break;
		}
	}
}

void Menu::handleEvent(SDL_Event& e, int& difficultyLevel, Difficulty& difficulty)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_1:
			difficultyLevel = 1;
			difficulty.barrelFrequency = 3;
			difficulty.barrelMovement = 1;
			difficulty.shipMovement = 2;
			difficulty.bombMovement = 2;
			difficulty.submarineMovement = 1;
			break;
		case SDLK_2:
			difficultyLevel = 2;
			difficulty.barrelFrequency = 2;
			difficulty.barrelMovement = 1;
			difficulty.shipMovement = 2;
			difficulty.bombMovement = 2;
			difficulty.submarineMovement = 2;
			break;
		case SDLK_3:
			difficultyLevel = 3;
			difficulty.barrelFrequency = 2;
			difficulty.barrelMovement = 2;
			difficulty.shipMovement = 1;
			difficulty.bombMovement = 1;
			difficulty.submarineMovement = 2;
			break;
		case SDLK_4:
			difficultyLevel = 4;
			difficulty.barrelFrequency = 1;
			difficulty.barrelMovement = 2;
			difficulty.shipMovement = 1;
			difficulty.bombMovement = 1;
			difficulty.submarineMovement = 3;
			break;
		default:
			break;
		}
	}
}

void Ship::move(Difficulty& difficulty)
{
	mPosX += mVelX * difficulty.shipMovement;
	mCollider.x = mPosX;

	if ((mPosX < 0) || (mPosX + SHIP_WIDTH > screen_width))
	{
		mPosX -= mVelX;
		mCollider.x = mPosX;
	}

	mCollider.y = mPosY;
}

void Bomb::move(Ship& ship, Difficulty& difficulty)
{
	if (activated) {
		mPosX = ship.mPosX;
		mPosY = ship.mPosY;
		activated = !activated;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;
	mPosY += mVelY * difficulty.bombMovement;

	if ((mPosY < 0) || (mPosY + BOMB_HEIGHT > screen_height))
	{
		mVelY = 0;
		mPosY = 1;
		mPosX = 1;
		mCollider.y = mPosY;
	}
}

void Submarine::move(Bomb& bomb, Difficulty& difficulty, int& gameStatus)
{
	if (facingRight) {
		mPosX += mVelX * difficulty.submarineMovement;
	}
	else {
		mPosX -= mVelX * difficulty.submarineMovement;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;

	if ((mPosX < 0) || (mPosX + SUBMARINE_WIDTH > screen_width))
	{
		mCollider.x = mPosX;
		const int changePosition = mPosY + rand() % 50 + (-25);
		if (changePosition < (screen_width - 20) && changePosition > 150)
		{
			mPosY = changePosition;
		}
		facingRight = !facingRight;
	}

	if (checkCollision(mCollider, bomb.mCollider)) {
		gameStatus = 3;
	}

	mCollider.y = mPosY;
}

void Barrel::move(Submarine& submarine, Ship& ship, SDL_Rect& waterSurface, int id, int& gameStatus, Difficulty& difficulty)
{
	if (!available)
	{
		if (activated) {
			mPosX = submarine.mPosX;
			mPosY = submarine.mPosY;
			mVelY = 1;
			activated = !activated;
		}

		acceleration += 1;
		if(acceleration == 10 || acceleration == 20 || acceleration == 30 || acceleration == 40)
		{
			velFromAcceleration = acceleration/10;
		}


		mCollider.x = mPosX;
		mCollider.y = mPosY;
		if (acceleration % 3 == 1) {
			mPosY -= (mVelY * difficulty.barrelMovement) + velFromAcceleration;
		}

		if (checkCollision(mCollider, ship.mCollider)) {
			gameStatus = 2;
		}
		else if (checkCollision(mCollider, waterSurface)) {
			mVelY = 0;
			mPosY = 800;
			mPosX = 800;
			acceleration = 0;
			velFromAcceleration = 0;
			available = true;

		}

	}
}

void Background::render() {
	gBackgroundSeaTexture.render(0, 0);
}

void Menu::render() {
	gMenuTexture.render(0, 0);
}


void WinScreen::render() {
	gWinScreenTexture.render(0, 0);
}

void LoseScreen::render() {
	gLoseScreenTexture.render(0, 0);
}


void Ship::render() const
{
	gShipTexture.render(mPosX, mPosY);
}

void Bomb::render() const
{
	gBombTexture.render(mPosX, mPosY);
}

void Submarine::render() const
{
	gSubmarineTexture.render(mPosX, mPosY);
}

void Barrel::render() const
{
	gBarrelTexture.render(mPosX, mPosY);
}

bool init()
{
	auto success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		gWindow = SDL_CreateWindow("Pawel Lakomiec s14094", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
		if (gWindow == nullptr)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == nullptr)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				const int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	bool success = true;

	if (!gBackgroundSeaTexture.loadFromFile("seafloor.bmp"))
	{
		printf("Failed to load seafloor texture!\n");
		success = false;
	}

	if (!gShipTexture.loadFromFile("battleShip.bmp"))
	{
		printf("Failed to load battleShip texture!\n");
		success = false;
	}

	if (!gBombTexture.loadFromFile("bomb.bmp"))
	{
		printf("Failed to load bomb texture!\n");
		success = false;
	}

	if (!gSubmarineTexture.loadFromFile("submarine.bmp"))
	{
		printf("Failed to load submarine texture!\n");
		success = false;
	}

	if (!gBarrelTexture.loadFromFile("barrel.bmp"))
	{
		printf("Failed to load barrel texture!\n");
		success = false;
	}

	if (!gWinScreenTexture.loadFromFile("winScreen.bmp"))
	{
		printf("Failed to load winScreen texture!\n");
		success = false;
	}

	if (!gLoseScreenTexture.loadFromFile("loseScreen.bmp"))
	{
		printf("Failed to load loseScreen texture!\n");
		success = false;
	}

	if (!gMenuTexture.loadFromFile("menuScreen.bmp"))
	{
		printf("Failed to load menu texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	gBackgroundSeaTexture.free();
	gShipTexture.free();
	gBombTexture.free();
	gSubmarineTexture.free();
	gBarrelTexture.free();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;
	gRenderer = nullptr;

	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//Calculate the sides of rect A
	const int leftA = a.x;
	const int rightA = a.x + a.w;
	const int topA = a.y;
	const int bottomA = a.y + a.h;

	//Calculate the sides of rect B
	const int leftB = b.x;
	const int rightB = b.x + b.w;
	const int topB = b.y;
	const int bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}


int main(int argc, char* args[])
{
	srand(time(nullptr));
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			bool quit = false;
			gameStatus = 0;
			difficultyLevel = 0;

			SDL_Event e;
			Menu menu;
			Difficulty difficulty;
			Background background;
			WinScreen win_screen;
			LoseScreen lose_screen;
			Ship ship;
			Bomb bomb;
			Submarine submarine;
			Barrel barrel[9];

			int bombTimer = 5000;
			unsigned int lastTime = 0;
			bool shot = false;

			SDL_Rect waterSurface;
			waterSurface.x = 0;
			waterSurface.y = 100;
			waterSurface.w = 640;
			waterSurface.h = 1;

			while (!quit)
			{
				if (gameStatus == 0)
				{
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					menu.render();
					if (difficultyLevel > 0)
					{
						gameStatus = 1;
					}
				}

				if (gameStatus == 1) {

					while (SDL_PollEvent(&e) != 0)
					{
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
						ship.handleEvent(e);
						bomb.handleEvent(e);
					}

					ship.move(difficulty);
					bomb.move(ship, difficulty);
					submarine.move(bomb, difficulty, gameStatus);

					const unsigned int currentTime = SDL_GetTicks();
					if (currentTime > lastTime + bombTimer) {
						bombTimer = 100 * (difficulty.barrelFrequency) + (difficulty.barrelFrequency * (rand() % 1000));
						lastTime = currentTime;

						for (auto& i : barrel)
						{
							if (i.available && !shot)
							{
								i.activated = true;
								i.available = false;
								shot = true;
							}
						}
						shot = false;

					}

			
					for(int i=0; i < 9; i++)
					{
						barrel[i].move(submarine, ship, waterSurface, i, gameStatus, difficulty);
					}


					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawRect(gRenderer, &waterSurface);

					background.render();
					ship.render();
					bomb.render();
					submarine.render();
					for (int i = 0; i < 9; i++)
					{
						barrel[i].render();
					}


				}

				if (gameStatus == 2)
				{
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					lose_screen.render();
				}
				if (gameStatus == 3) {
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					win_screen.render();
				}
				SDL_RenderPresent(gRenderer);

			}
		}
	}

	close();
	return 0;
}