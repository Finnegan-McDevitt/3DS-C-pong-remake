#include <citro2d.h>
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 400
#define SCREEN_HIGHT 240

#define PADDLE_HIGHT 80
#define PADDLE_HITBOX_THICKNES 10
#define PLAYER_X_POS 320
#define COMP_X_POS 60
#define PLAYER_SPEED 4

#define BALL_DEFAULT_SPEED 2
#define PADDLE_DEFAULT_POS 80
#define BALL_DEFAULT_X SCREEN_WIDTH / 2;
#define BALL_DEFAULT_Y SCREEN_HIGHT / 2;


void BallBouncePlayer(float *ballSpeedX, float ballXPos, float ballYPos, float playerY)
{
	if (((ballXPos >= PLAYER_X_POS) && (ballXPos <= (PLAYER_X_POS + PADDLE_HITBOX_THICKNES))) && ((ballYPos >= playerY) && (ballYPos <= (playerY + PADDLE_HIGHT))))
	{
		float distFromCenter = ballYPos - (playerY + (.5f * PADDLE_HIGHT));
		float mult = distFromCenter / (.5f * PADDLE_HIGHT);
		mult = fabsf(mult);
		printf("\x1b[16;1HLast Dist from Center Mult = %.4f", mult);
		*ballSpeedX = -BALL_DEFAULT_SPEED * mult;
	}
}

void BallBounceComp(float *ballSpeedX, float ballXPos, float ballYPos, float compY)
{
	if (((ballXPos <= COMP_X_POS) && (ballXPos >= (COMP_X_POS - PADDLE_HITBOX_THICKNES))) && ((ballYPos >= compY) && (ballYPos <= (compY + PADDLE_HIGHT))))
	{
		*ballSpeedX = BALL_DEFAULT_SPEED;
	}
}

static C2D_Text *MakeText(int score, C2D_Font font, C2D_TextBuf buff)
{
	C2D_Text result;
	C2D_TextBufClear(buff);
	char text[12];
	snprintf(text, sizeof(text), "%d", score);
	C2D_TextFontParse(&result, font, buff, text);
	C2D_TextOptimize(&result);
	if(!font)
	{
		printf("\x1b[13;1HChar Array = %s, Text Buffer = %s, Buffer of Text object = %s", text, buff, result.buf);
	}
	//result.font = NULL;
	return &result;
}

/*
void ChangeDificulty(float *compSpeed) 
{
	static SwkbdState swkbd;
	static char mybuf[60];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;


}*/


int main(int argc, char **argv)
{
	// Initialize services
	gfxInitDefault();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_BOTTOM, NULL);

	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

	u32 clrClear = C2D_Color32(0xF0, 0xBC, 0x2B, 0xFF);

	//u32 clrBlack = C2D_Color32(0x00, 0x00, 0x00, 0xFF);

	C2D_Font font = C2D_FontLoadSystem(CFG_REGION_USA);
	C2D_TextBuf buff = C2D_TextBufNew(256);


	//u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0; //In these variables there will be information about keys detected in the previous frame

	printf("\x1b[1;1HPress A to Start. Press Start to exit.");
	printf("\x1b[2;1HCirclePad position:");
	printf("\x1b[27;1HBy Finnegan McDevitt");

	float playerY = 80;

	float compY = 80;

	float comp_speed = 1.5;

	float ballXPos = BALL_DEFAULT_X;
	float ballYPos = BALL_DEFAULT_Y;
	float ballSpeedX = BALL_DEFAULT_SPEED;
	float ballSpeedY = BALL_DEFAULT_SPEED;

	int playerScore = 0;
	int compScore = 0;

	bool isPlaying = false;


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
		

		C2D_Text *playerScoreText = MakeText(playerScore, font, buff);
		//Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrClear);
		C2D_SceneBegin(top);



		printf("\x1b[3;1H%04d %04d", pos.dx, pos.dy);

		printf("\x1b[4;1HPlayer Y = %.2f", playerY);

		printf("\x1b[5;1HBall X = %.2f, Y = %.2f", ballXPos, ballYPos);

		printf("\x1b[6;1HBall Speed X = %.2f, Y = %.2f", ballSpeedX, ballSpeedY);

		printf("\x1b[7;1HPlayer Score = %d, Computer Score = %d", playerScore, compScore);

		printf("\x1b[10;1HCPU:     %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f);
		printf("\x1b[11;1HGPU:     %6.2f%%\x1b[K", C3D_GetDrawingTime()*6.0f);
		printf("\x1b[12;1HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage()*100.0f);

		if (!isPlaying)
		{
			if (kDown & KEY_A) 
			{
				isPlaying = true;
			}
			ballXPos = BALL_DEFAULT_X;
			ballYPos = BALL_DEFAULT_Y;
			ballSpeedX = BALL_DEFAULT_SPEED;
			ballSpeedY = BALL_DEFAULT_SPEED;
			
			playerY = PADDLE_DEFAULT_POS;
			compY = PADDLE_DEFAULT_POS;

		}

		if (isPlaying)
		{
			u32 currentButton64 = 64;
			u32 currentButton128 = 128;
			if ((pos.dy > 0 || (kDown & currentButton64) || (kHeld & currentButton64)) && (playerY > 0))
			{
				playerY -= PLAYER_SPEED;
			}
			if ((pos.dy < 0 || (kDown & currentButton128) || (kHeld & currentButton128)) && (playerY < (240 - PADDLE_HIGHT)))
			{
				playerY += PLAYER_SPEED;
			}

			//ball mechinacs
			ballXPos += ballSpeedX;
			ballYPos += ballSpeedY;

			if (ballYPos >= 230)
			{
				ballSpeedY = -BALL_DEFAULT_SPEED;
			}
			else if (ballYPos <= 0)
			{
				ballSpeedY = BALL_DEFAULT_SPEED;
			}

			if (ballXPos >= 390)
			{
				ballSpeedX = -BALL_DEFAULT_SPEED;
				compScore++;
				isPlaying = false;
			}
			else if (ballXPos <= 0)
			{
				ballSpeedX = BALL_DEFAULT_SPEED;
				playerScore++;
				isPlaying = false;
			}

			BallBouncePlayer(&ballSpeedX, ballXPos, ballYPos, playerY);
			

			if ((compY + (.5 * PADDLE_HIGHT)) < ballYPos) {
				compY += comp_speed;
			}
			else if ((compY + (.5 * PADDLE_HIGHT)) > ballYPos)
			{
				compY -= comp_speed;
			}

			BallBounceComp(&ballSpeedX, ballXPos, ballYPos, compY);
		}
		


		//Shapes Drawing and moving
		C2D_DrawRectSolid(PLAYER_X_POS + 10, playerY, 0, 10, PADDLE_HIGHT, clrWhite);

		C2D_DrawRectSolid(COMP_X_POS - 10, compY, 0, 10, PADDLE_HIGHT, clrWhite);

		C2D_DrawRectSolid(ballXPos, ballYPos, 0, 10, 10, clrWhite);

		C2D_DrawText(playerScoreText, C2D_WithColor, 10, 10, 0, 10, 10, clrWhite);

		C3D_FrameEnd(0);

		if(kDown & KEY_B)
		{
			//ChangeDificulty(&comp_speed);
		}


		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxScreenSwapBuffers(GFX_BOTTOM, false);

		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	C2D_TextBufDelete(buff);
	C2D_FontFree(font);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
