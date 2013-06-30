#include "SDL.h"
#include "SDL_image.h"
#include "SDL_TTF.h"
#include "SDL_mixer.h"
#include <string>
#include <sstream>
using namespace std;

//Constants
#pragma region Constants

const int SCREEN_WIDTH = 531;
const int SCREEN_HEIGHT = 412;
const int SCREEN_BPP = 32;

const int FPS = 20;

const int PADDLE_HEIGHT = 53;
const int PADDLE_WIDTH = 7;

const int BALL_HEIGHT = 9;
const int BALL_WIDTH = 9;

#pragma endregion Constants

//Global Variables
#pragma region Globals

SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;

SDL_Surface* paddle = NULL;
SDL_Surface* ball = NULL;

SDL_Surface* p1ScoreSDL = NULL;
SDL_Surface* p2ScoreSDL = NULL;
int scoreInt1 = 0;
int scoreInt2 = 0;

Mix_Chunk* beep = NULL;
Mix_Chunk* boop = NULL;

SDL_Rect top;
SDL_Rect bottom; 

TTF_Font* font = NULL;

SDL_Color textColor = {255, 255, 255};

TTF_Font* title = NULL;
SDL_Rect titleBackground;
SDL_Surface* titleSubText = NULL;

SDL_Event event;
#pragma endregion Globals

#pragma region Necessary Functions

/*Initialize Function that starts up SDL, and all the other necessary
  funtionality for the program */

bool init()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

	if(TTF_Init() == -1)
	{
		return false;
	}

	if(screen == NULL)
	{
		return false;
	}

	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		return false;
	}

	srand(SDL_GetTicks());

	SDL_WM_SetCaption("Pong", NULL);

	return true;
}

/*Loads and Optimizes images that were loaded from files*/

SDL_Surface* load_images(std::string filename)
{
	SDL_Surface* loadedImage = NULL;
	SDL_Surface* optimizedImage = NULL;

	loadedImage = IMG_Load(filename.c_str());

	if(loadedImage != NULL)
	{
		optimizedImage = SDL_DisplayFormat(loadedImage);
		SDL_FreeSurface(loadedImage);

		if(optimizedImage != NULL)
		{
			SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB(optimizedImage->format, 0, 0xFF, 0xFF));
		}
	}

	return optimizedImage;
}

//Loads all the necessary files

bool load_files()
{
	background = load_images("background.png");
	paddle = load_images("paddle.png");
	ball = load_images("ball.png");
	font = TTF_OpenFont("press.ttf", 15);
	title = TTF_OpenFont("press.ttf", 50);
	beep  = Mix_LoadWAV("BEEP.OGG");
	boop = Mix_LoadWAV("BOOP.OGG");

	if(background == NULL || paddle == NULL || ball == NULL || font == NULL || beep == NULL || boop == NULL)
	{
		return false;
	}

	return true;
}

//Applies a source image to a destination and Blits it
void apply(int X, int Y, SDL_Surface* source, SDL_Surface* dest, SDL_Rect* clip = NULL)
{
	SDL_Rect offset;

	offset.x = X;
	offset.y = Y;

	SDL_BlitSurface(source, clip, dest, &offset);
}

#pragma endregion Necessary Functions

#pragma region Game Specific Functions

//Function that creates the game board and resets the score
void create_game()
{
	top.x = 0;
	top.y = 0;
	top.w = SCREEN_WIDTH;
	top.h = 24;

	bottom.x = 0;
	bottom.y = 390;
	bottom.w = SCREEN_WIDTH;
	bottom.h = 50;

	scoreInt1 = 0;
	scoreInt2 = 0;
}

//Collision Detection
bool check_collision(SDL_Rect A, SDL_Rect B)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = A.x;
	rightA = A.x + A.w;
	topA = A.y;
	bottomA = A.y + A.h;

	leftB = B.x;
	rightB = B.x + B.w;
	topB = B.y;
	bottomB = B.y + B.h;

	if(bottomA <= topB)
	{
		return false;
	}

	if(topA >= bottomB)
	{
		return false;
	}

	if(leftA >= rightB)
	{
		return false;
	}

	if(rightA <= leftB)
	{
		return false;
	}

	return true;
}

//Adds a point to the players score
void score(int player)
{
	if(player == 1)
	{
		scoreInt1++;
	}

	if(player == 2)
	{
		scoreInt2++;
	}
}

#pragma endregion Game Specific Functions

#pragma region Classes

//Timer class and timer functions to keep track of FPS
#pragma region Timer

class Timer
{
private:
	int startTicks;
	int pausedTicks;
	bool paused;
	bool started;

public:
	Timer();

	void start();
	void stop();
	void pause();
	void unpause();

	int get_ticks();

	bool is_started();
	bool is_paused();
};

Timer::Timer()
{
	startTicks = 0;
	pausedTicks = 0;
	paused = false;
	started = false;

}

#pragma region Timer Functions

void Timer::start()
{
	started = true;
	paused = false;

	startTicks = SDL_GetTicks();
}

void Timer::stop()
{
	started = false;
	paused = false;
}

void Timer::pause()
{
	if((started == true) && (paused == false))
	{
		paused = true;
		pausedTicks = SDL_GetTicks() - startTicks;
	}
}

void Timer::unpause()
{
	if(paused == true)
	{
		paused = false;
		startTicks = SDL_GetTicks() - pausedTicks;
		pausedTicks = 0;
	}
}

int Timer::get_ticks()
{
	if(started == true)
	{
		if(paused == true)
		{
			return pausedTicks;
		}

		else
		{
			return SDL_GetTicks() - startTicks;
		}
	}

	return 0;
}

bool Timer::is_started()
{
	return started;
}

bool Timer::is_paused()
{
	return paused;
}

#pragma endregion Timer Functions

#pragma endregion Timer

//Paddle class and functions to work the paddle
#pragma region Paddle

class Paddle
{
private:

	int xVel, yVel;
	

public:
	SDL_Rect paddleCollider;
	Paddle(int player);

	void handle_input(int player);
	void move();
	void show();
	int getY();

};

Paddle::Paddle(int player)
{

	if(player == 1)
	{
	paddleCollider.x = 500;
	}

	else
	{
	paddleCollider.x = 30;
	}

	paddleCollider.y = SCREEN_HEIGHT/2;
	paddleCollider.w = PADDLE_WIDTH;
	paddleCollider.h = PADDLE_HEIGHT;

	xVel = 0;
	yVel = 0;


}

#pragma region Paddle Functions

void Paddle::move()
{
	paddleCollider.y += yVel;

	if(check_collision(paddleCollider, top) || (check_collision(paddleCollider, bottom)))
	{
		paddleCollider.y -= yVel;
	}
}

void Paddle::show()
{
	apply(paddleCollider.x, paddleCollider.y, paddle, screen);
}

void Paddle::handle_input(int player)
{

	if(player == 1)
	{

		if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
			case SDLK_UP: yVel -= PADDLE_HEIGHT/4; break;
			case SDLK_DOWN: yVel += PADDLE_HEIGHT/4; break;
			}
		}

		else if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_UP: yVel += PADDLE_HEIGHT/4; break;
			case SDLK_DOWN: yVel -= PADDLE_HEIGHT/4; break;
			}
		}
	}

	else 
	{
		if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
			case SDLK_w: yVel -= PADDLE_HEIGHT/4; break;
			case SDLK_s: yVel += PADDLE_HEIGHT/4; break;
			}
		}

		else if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_w: yVel += PADDLE_HEIGHT/4; break;
			case SDLK_s: yVel -= PADDLE_HEIGHT/4; break;
			}
		}
	}
}

int Paddle::getY()
{
	return paddleCollider.y;
}

#pragma endregion Paddle Functions

#pragma endregion Paddle

//Ball class. Creates and operates the ball
#pragma region Ball 

class Ball
{
private:
	int xVel, yVel;
	SDL_Rect ballCollider;

public:

	bool moving;
	int xDir, yDir;
	Ball();

	void move(SDL_Rect paddle);
	void show();
	int getY();
};

Ball::Ball()
{
	moving = false;
	ballCollider.x = SCREEN_WIDTH/2;
	ballCollider.y = SCREEN_HEIGHT/2;
	ballCollider.w = BALL_WIDTH;
	ballCollider.h = BALL_HEIGHT;

		xVel = 4;
		yVel = 4;

		xDir = 1;
		yDir = 1;
};

#pragma region Ball Functions

void Ball::move(SDL_Rect paddle)
{

	int random = rand() % 2;
	int random2 = rand()% 100;

	if(!moving)
	{
		if(random == 0)
		{
			xDir = 1;
			
			if(random2 >= 50)
			{
			yDir = 1;
			}

			else
			{
			yDir = -1;
			}

			moving = true;
		}

		else
		{
			xDir = -1;
	
			if(random2 < 50)
			{
			yDir = -1;
			}

			else
			{
			yDir = 1;
			}
			moving = true;
		}
	}

	if(xDir == 1)
	{
	ballCollider.x += xVel;
	}
	
	if(xDir == -1)
	{
		ballCollider.x -= xVel;
	}

	if(yDir == 1)
	{
		ballCollider.y += yVel;
	}

	if(yDir == -1)
	{
		ballCollider.y -= yVel;
	}

	if(check_collision(ballCollider, top))
	{
		Mix_PlayChannel(-1, beep, 0);
		yDir = 1;
		ballCollider.y += yVel;
	}

	 if(check_collision(ballCollider, bottom))
	 {
		 Mix_PlayChannel(-1, beep, 0);
		 yDir = -1;
		 ballCollider.y -= yVel;
	 }

	 if(xDir == 1)
	 {
		 if(check_collision(ballCollider, paddle))
		 {
			 Mix_PlayChannel(-1, beep, 0);
			 xDir = -1;
			 ballCollider.x -= xVel;
		 }
	 }

	 if(xDir == -1)
	 {
		 if(check_collision(ballCollider, paddle))
		 {
			 Mix_PlayChannel(-1, beep, 0);
			 xDir = 1;
			 ballCollider.x += xVel;
		 }
	 }

	 if(ballCollider.x > SCREEN_WIDTH + 5)
	 {
		 Mix_PlayChannel(-1, boop, 2);
		 ballCollider.x = SCREEN_WIDTH/2;
		 score(2);
		 moving = false;
	 }

	 if(ballCollider.x < -5)
	 {
		 Mix_PlayChannel(-1, boop, 2);
		 ballCollider.x = SCREEN_WIDTH/2;
		 score(1);
		 moving = false;
	 }
} 

void Ball::show()
{
	apply(ballCollider.x, ballCollider.y, ball, screen);
}

int Ball::getY()
{
	return ballCollider.y;
}

#pragma endregion Ball Functions

#pragma endregion Ball 

#pragma endregion Classes

//Function that closes everything down and cleans up the file
#pragma region Clean Up

void clean_up()
{

SDL_FreeSurface(paddle);
SDL_FreeSurface(ball);
SDL_FreeSurface(background);

TTF_CloseFont(font);
TTF_Quit();

Mix_FreeChunk(beep);
Mix_FreeChunk(boop);

Mix_CloseAudio();
SDL_Quit();

}

#pragma endregion Clean Up

int main(int argc, char* args[])
{

	Timer fps;
	Paddle player1(1);
	Paddle player2(2);
	Ball myBall;

	bool quit = false;

	std::stringstream score1;
	std::stringstream score2;

	create_game();

	init();
	load_files();

	while(quit == false)
	{
		fps.start();

		while(SDL_PollEvent(&event))
		{

			player1.handle_input(1);
			player2.handle_input(2);

			if(event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		score1.str(std::string());
		score1 << scoreInt1;
		p1ScoreSDL = TTF_RenderText_Solid(font, score1.str().c_str(), textColor);

		score2.str(std::string());
		score2 << scoreInt2;
		p2ScoreSDL = TTF_RenderText_Solid(font, score2.str().c_str(), textColor);


		apply(0,0, background, screen);
		
		apply(SCREEN_WIDTH/2 + 27, 40, p1ScoreSDL, screen);

		apply(SCREEN_WIDTH/2 - 40, 40, p2ScoreSDL, screen);

		player1.move();
		player1.show();
		player2.move();
		player2.show();

		if(myBall.xDir == 1)
		{
			myBall.move(player1.paddleCollider);
		}
		
		if(myBall.xDir == -1)
		{
			myBall.move(player2.paddleCollider);
		}

		myBall.show();
		
		SDL_Flip(screen);

		if(fps.get_ticks() < 1000/FPS)
		{
			SDL_Delay((1000/FPS) - fps.get_ticks());
		}

	}

	clean_up();
	return 0;
}