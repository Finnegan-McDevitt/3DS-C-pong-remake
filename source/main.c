#include <citro2d.h>
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//display defines
#define SCREEN_WIDTH 400
#define SCREEN_HIGHT 240

//player defines
#define PADDLE_HIGHT 80
#define PADDLE_HITBOX_THICKNES 10
#define PLAYER_X_POS 320
#define COMP_X_POS 60
#define PLAYER_SPEED 4
#define PLAYER_START_Y 80

//ball defines
#define BALL_SIZE 10
#define BALL_DEFAULT_SPEED 2
#define PADDLE_DEFAULT_POS 80
#define BALL_DEFAULT_X (SCREEN_WIDTH / 2) - (BALL_SIZE / 2)
#define BALL_DEFAULT_Y (SCREEN_HIGHT / 2) - (BALL_SIZE / 2)

//score display defines
#define PLAYER_SCORE_X 280
#define COM_SCORE_X 100
#define SCORE_Y 20
#define SCORE_MAX_CHARS 12

//stereoscopic 3d defines
//max number of pixels the paddles are shifted per eye when the 3d slider is all the way up.
//keep around 4-6
#define MAX_POPOUT 6.0f
//the ball pops out a bit further than the paddles
#define BALL_POPOUT_MULT 1.2f
//global variables

//colors and score text, shared with DrawScene
u32 clrWhite;
u32 clrBlack;
C2D_Text playerScoreText;
C2D_Text comScoreText;

//player positions
float playerY = PLAYER_START_Y;
float compY = PLAYER_START_Y;

//computer dificulty
float dificulty = 1;

//ball position, initalized to the defaults
float baseBallSpeed = BALL_DEFAULT_SPEED;
float ballXPos = BALL_DEFAULT_X;
float ballYPos = BALL_DEFAULT_Y;
float ballSpeedX = BALL_DEFAULT_SPEED;
float ballSpeedY = BALL_DEFAULT_SPEED;

//scores
int playerScore = 0;
int comScore = 0;

bool isPlaying = false;
int last_player_score = -1;
int last_com_score = -1;


void BallBouncePlayer(float *ballSpeedX, float ballXPos, float ballYPos, float playerY)
{
	if (((ballXPos >= PLAYER_X_POS) && (ballXPos <= (PLAYER_X_POS + PADDLE_HITBOX_THICKNES))) && ((ballYPos >= playerY) && (ballYPos <= (playerY + PADDLE_HIGHT))))
	{
		float distFromCenter = ballYPos - (playerY + (.5f * PADDLE_HIGHT));
		float mult = distFromCenter / (.5f * PADDLE_HIGHT);
		mult = fabsf(mult);
		printf("\x1b[16;1HLast Dist from Center Mult = %.4f", mult);
		*ballSpeedX = -baseBallSpeed * mult;
	}
}

void BallBounceComp(float *ballSpeedX, float ballXPos, float ballYPos, float compY)
{
	if (((ballXPos <= COMP_X_POS) && (ballXPos >= (COMP_X_POS - PADDLE_HITBOX_THICKNES))) && ((ballYPos >= compY) && (ballYPos <= (compY + PADDLE_HIGHT))))
	{
		*ballSpeedX = baseBallSpeed;
	}
}

static void MakeText(int score, C2D_Font *font, C2D_TextBuf buff, C2D_Text* result)
{
	C2D_TextBufClear(buff);
	char text[SCORE_MAX_CHARS];
	snprintf(text, sizeof(text), "%d", score);
	C2D_TextFontParse(result, *font, buff, text);
	C2D_TextOptimize(result);
	
	if(!*font)
	{
		printf("\x1b[14;1HNo Font,");
	} else {
		printf("\x1b[14;1HFont,");
	}
	printf("\x1b[14;9HChar Array = %s", text);
	return;
	
}


void ChangeDificulty() 
{
	printf("\x1b[25;1HKeyboard Starting");

	//init keyboard values
	static SwkbdState swkbd;
	static char buff[60];
	//static SwkbdStatusData swkbdStatus;
	//static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;

	 

	//init the keyboard info
	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, 60);
	swkbdSetNumpadKeys(&swkbd, '.', 0); //add a decimal point key to the numpad (0 = no key on the right)
	swkbdSetHintText(&swkbd, "Enter computer speed multiplier. Default is 1");
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	
	//create the keyboard
	button = swkbdInputText(&swkbd, buff, sizeof(buff));

	//make sure user did not cancel, and then apply change
	if (button != SWKBD_BUTTON_NONE && button != SWKBD_BUTTON_LEFT) {
		dificulty = atof(buff);
		baseBallSpeed = BALL_DEFAULT_SPEED * dificulty;
	}
}

//draws the whole top screen for one eye.
//eye is the number of pixels to shift the pop-out objects horizontally:
//positive for the left eye, negative for the right eye, so the object appears in front of the screen.
//the halfway line and the score text are drawn at the same spot for both eyes so they stay flat on the screen.
static void DrawScene(float eye)
{
	//draw halfway line (no 3d)
	C2D_DrawRectSolid(199.5f, 0.0f, 0.0f, 2.0f, 240.0f, clrBlack);

	//Shapes Drawing and moving
	//draw player
	C2D_DrawRectSolid(PLAYER_X_POS + 10 + eye, playerY, 0, 10, PADDLE_HIGHT, clrWhite);

	//draw computer
	C2D_DrawRectSolid(COMP_X_POS - 10 + eye, compY, 0, 10, PADDLE_HIGHT, clrWhite);

	//draw ball, pops out further than the paddles
	C2D_DrawRectSolid(ballXPos + (eye * BALL_POPOUT_MULT), ballYPos, 0, BALL_SIZE, BALL_SIZE, clrWhite);

	//draw player score (no 3d)
	C2D_DrawText(&playerScoreText, C2D_WithColor, PLAYER_SCORE_X, SCORE_Y, 1.0f, 1.0f, 1.0f, clrBlack);

	//draw com score (no 3d)
	C2D_DrawText(&comScoreText, C2D_WithColor, COM_SCORE_X, SCORE_Y, 1.0f, 1.0f, 1.0f, clrBlack);
}


int main(int argc, char **argv)
{
	// Initialize services
	gfxInitDefault();
	//turn on stereoscopic 3d for the top screen
	gfxSet3D(true);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_BOTTOM, NULL);

	//one render target per eye. the scene is drawn twice per frame, with the pop-out
	//objects shifted right for the left eye and left for the right eye
	C3D_RenderTarget* topLeft = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* topRight = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);

	clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

	u32 clrClear = C2D_Color32(0xF0, 0xBC, 0x2B, 0xFF);

	clrBlack = C2D_Color32(0x00, 0x00, 0x00, 0xFF);

	//load the North American region system font
	C2D_Font font = C2D_FontLoadSystem(CFG_REGION_USA);
	// if (!font) {
	// 	printf("\x1b[26;1HFont is NULL");
	// }
	
	//create buffers for player score display
	C2D_TextBuf playeScoreBuff = C2D_TextBufNew(SCORE_MAX_CHARS);
	MakeText(playerScore, &font, playeScoreBuff, &playerScoreText);

	//create buffers for com score display
	C2D_TextBuf comScoreBuf = C2D_TextBufNew(SCORE_MAX_CHARS);
	MakeText(comScore, &font, comScoreBuf, &comScoreText);



	//u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0; //In these variables there will be information about keys detected in the previous frame

	printf("\x1b[1;1HPress A to Start. Press Start to exit.");
	printf("\x1b[2;1HCirclePad position:");
	printf("\x1b[27;1HBy Finnegan McDevitt");




	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		//hidKeysHeld returns information about which buttons have are held down in this frame
		u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		circlePosition pos;

		//Read the CirclePad position
		hidCircleRead(&pos);

		//Print the CirclePad position
		
		//display the score
		if (last_player_score != playerScore){
			MakeText(playerScore, &font, playeScoreBuff, &playerScoreText);
			last_player_score = playerScore;
		}

		if (last_com_score != comScore){
			MakeText(comScore, &font, comScoreBuf, &comScoreText);
			last_com_score = comScore;
		}
		
		//check if player wants to change com speed
		if(kDown & KEY_B)
		{
			ChangeDificulty();
		} else {
			printf("\x1b[25;1H                 ");
		}
		
		
		printf("\x1b[3;1H%04d %04d", pos.dx, pos.dy);

		printf("\x1b[4;1HPlayer Y = %.2f", playerY);

		printf("\x1b[5;1HBall X = %.2f, Y = %.2f", ballXPos, ballYPos);

		printf("\x1b[6;1HBall Speed X = %.2f, Y = %.2f", ballSpeedX, ballSpeedY);

		printf("\x1b[7;1HPlayer Score = %d, Computer Score = %d", playerScore, comScore);

		printf("\x1b[10;1HCPU:     %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f);
		printf("\x1b[11;1HGPU:     %6.2f%%\x1b[K", C3D_GetDrawingTime()*6.0f);
		printf("\x1b[12;1HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage()*100.0f);

		printf("\x1b[24;1HCOM speed = %f", dificulty);

		
		if (isPlaying)
		{

			if (((kDown & KEY_UP) || (kHeld & KEY_UP)) && (playerY > 0))
			{
				playerY -= PLAYER_SPEED;
			}
			if (((kDown & KEY_DOWN) || (kHeld & KEY_DOWN)) && (playerY < (240 - PADDLE_HIGHT)))
			{
				playerY += PLAYER_SPEED;
			}

			//ball mechinacs
			ballXPos += ballSpeedX;
			ballYPos += ballSpeedY;

			if (ballYPos >= 230)
			{
				ballSpeedY = -baseBallSpeed;
			}
			else if (ballYPos <= 0)
			{
				ballSpeedY = baseBallSpeed;
			}

			if (ballXPos >= 390)
			{
				ballSpeedX = -baseBallSpeed;
				comScore++;
				isPlaying = false;
			}
			else if (ballXPos <= 0)
			{
				ballSpeedX = baseBallSpeed;
				playerScore++;
				isPlaying = false;
			}

			BallBouncePlayer(&ballSpeedX, ballXPos, ballYPos, playerY);
			

			if ((compY + (.5 * PADDLE_HIGHT)) < ballYPos) {
				compY += dificulty * 1.5;
			}
			else if ((compY + (.5 * PADDLE_HIGHT)) > ballYPos)
			{
				compY -= dificulty * 1.5;
			}

			BallBounceComp(&ballSpeedX, ballXPos, ballYPos, compY);
		} else {
			if (kDown & KEY_A) 
			{
				isPlaying = true;
			}
			ballXPos = BALL_DEFAULT_X;
			ballYPos = BALL_DEFAULT_Y;
			ballSpeedX = baseBallSpeed;
			ballSpeedY = baseBallSpeed;
			
			playerY = PADDLE_DEFAULT_POS;
			compY = PADDLE_DEFAULT_POS;

		}


		//read the 3d slider (0 = off, 1 = all the way up) and turn it into a pixel shift
		float slider = osGet3DSliderState();
		float depth = slider * MAX_POPOUT;
		printf("\x1b[8;1H3D slider = %.2f, depth = %.2f px", slider, depth);

		//Render the scene, once per eye
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		//left eye, pop-out objects shifted right
		C2D_TargetClear(topLeft, clrClear);
		C2D_SceneBegin(topLeft);
		DrawScene(depth);

		//right eye, pop-out objects shifted left.
		//skipped when the slider is off so the game runs at full speed in 2d
		if (depth > 0.0f)
		{
			C2D_TargetClear(topRight, clrClear);
			C2D_SceneBegin(topRight);
			DrawScene(-depth);
		}

		C3D_FrameEnd(0);

		


		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	C2D_TextBufDelete(playeScoreBuff);
	C2D_TextBufDelete(comScoreBuf);
	C2D_FontFree(font);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
