
//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <windows.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

//The ship that will move around on the screen
class Ship
{
public:
	//The dimensions of the ship
	static const int SHIP_WIDTH = 40;
	static const int SHIP_HEIGHT = 20;

	//Maximum axis velocity of the ship
	static const int SHIP_VEL = 1;

	//Initializes the variables
	Ship();

	//Takes key presses and adjusts the ship's velocity
	void handleEvent(SDL_Event& e);

	//Moves the ship and checks collision
	void move(SDL_Rect& waterSurface);

	//Shows the ship on the screen
	void render();

private:
	//The X and Y offsets of the ship
	int mPosX, mPosY;

	//The velocity of the ship
	int mVelX, mVelY;

	//Ship's collision box
	SDL_Rect mCollider;
};

class Submarine
{
public:
	//The dimensions of the ship
	static const int SUBMARINE_WIDTH = 27;
	static const int SUBMARINE_HEIGHT = 20;

	//Maximum axis velocity of the ship
	static const int SUBMARINE_VEL = 2;

	//Initializes the variables
	Submarine();

	//Takes key presses and adjusts the ship's velocity
	void handleEvent(SDL_Event& e);

	//Moves the ship and checks collision
	void move(SDL_Rect& waterSurface);

	void move(bool & facingRight);

	//Shows the ship on the screen
	void render();

private:
	//The X and Y offsets of the ship
	int mPosX, mPosY;

	//The velocity of the ship
	int mVelX, mVelY;

	//Ship's collision box
	SDL_Rect mCollider;
};


//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Box collision detector
bool checkCollision(SDL_Rect a, SDL_Rect b);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gShipTexture;

//Scene textures
LTexture gSubmarineTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
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

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
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

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Ship::Ship()
{
	//Initialize the offsets
	mPosX = 0;
	mPosY = 80;

	//Set collision box dimension
	mCollider.w = SHIP_WIDTH;
	mCollider.h = SHIP_HEIGHT;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;
}

Submarine::Submarine()
{
	//Initialize the offsets
	mPosX = 0;
	mPosY = 400;

	//Set collision box dimension
	mCollider.w = SUBMARINE_WIDTH;
	mCollider.h = SUBMARINE_HEIGHT;

	//Initialize the velocity
	mVelX = 2;
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
		case SDLK_UP: mVelY -= SHIP_VEL; break;
		case SDLK_DOWN: mVelY += SHIP_VEL; break;
		case SDLK_LEFT: mVelX -= SHIP_VEL; break;
		case SDLK_RIGHT: mVelX += SHIP_VEL; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += SHIP_VEL; break;
		case SDLK_DOWN: mVelY -= SHIP_VEL; break;
		case SDLK_LEFT: mVelX += SHIP_VEL; break;
		case SDLK_RIGHT: mVelX -= SHIP_VEL; break;
		}
	}
}

void Ship::move(SDL_Rect& waterSurface)
{
	//Move the ship left or right
	mPosX += mVelX;
	mCollider.x = mPosX;


	//If the ship collided or went too far to the left or right
	if ((mPosX < 0) || (mPosX + SHIP_WIDTH > SCREEN_WIDTH) || checkCollision(mCollider, waterSurface))
	{
		//Move back
		mPosX -= mVelX;
		mCollider.x = mPosX;
	}

	//Move the ship up or down

	//mPosY += mVelY;
	mCollider.y = mPosY;

	//If the ship collided or went too far up or down
	if ((mPosY < 0) || (mPosY + SHIP_HEIGHT > SCREEN_HEIGHT) || checkCollision(mCollider, waterSurface))
	{
		//Move back
		mPosY -= mVelY;
		mCollider.y = mPosY;
	}
}

void Submarine::move(bool& facingRight)
{
	if (facingRight) {
		mPosX += mVelX;
	}
	else {
		mPosX -= mVelX;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;

	if ((mPosX < 0) || (mPosX + SUBMARINE_WIDTH > SCREEN_WIDTH))
	{
		//mPosX -= mVelX;
		mCollider.x = mPosX;
		facingRight = !facingRight;
	}

	mCollider.y = mPosY;
}

void Ship::render()
{
	gShipTexture.render(mPosX, mPosY);
}

void Submarine::render()
{
	gSubmarineTexture.render(mPosX, mPosY);
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
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

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
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
	//Loading success flag
	bool success = true;

	//Load press texture
	if (!gShipTexture.loadFromFile("battleShip.bmp"))
	{
		printf("Failed to load ship texture!\n");
		success = false;
	}

	//Load press texture
	if (!gSubmarineTexture.loadFromFile("submarine.bmp"))
	{
		printf("Failed to load submarine texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gShipTexture.free();
	gSubmarineTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

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
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The ship that will be moving around on the screen
			Ship ship;

			Submarine submarine;

			bool facingRight = true;

			//Set the waterSurface
			SDL_Rect waterSurface;
			waterSurface.x = 0;
			waterSurface.y = 100;
			waterSurface.w = 640;
			waterSurface.h = 1;

			//While application is running
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					//Handle input for the ship
					ship.handleEvent(e);
				}

				//Move the ship and check collision
				ship.move(waterSurface);
				submarine.move(facingRight);

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render waterSurface
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderDrawRect(gRenderer, &waterSurface);

				//Render ship
				ship.render();

				submarine.render();

				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}