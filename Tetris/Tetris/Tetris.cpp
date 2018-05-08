#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
using namespace std;

wstring tetronimo[7];
int fieldWidth = 12;
int fieldHeight = 18;
unsigned char *field = nullptr;

int screenWidth = 120;
int screenHeight = 30;

// Rotation adjustment based on a 4x4 grid
int Rotate(int x, int y, int rotation)
{
	switch (rotation % 4)
	{
		case 0: return y * 4 + x;			// 0 degrees
		case 1: return 12 + y - (x * 4);	// 90 degrees
		case 2: return 15 - (y * 4) - x;	// 180 degrees
		case 3: return 3 - y + (x * 4);		// 270 degrees
	}
	return 0;
}


bool DoesPieceFit(int curTetronimo, int curRotation, int PosX, int PosY)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			// Index for piece
			int pi = Rotate(x, y, curRotation);
			// Index for field
			int fi = (PosY + y) * fieldWidth + (PosX + x);

			// If we're within bounds, so not to check out of range
			if (PosX + x >= 0 && PosX + x < fieldWidth)
			{
				if (PosY + y >= 0 && PosY + x < fieldHeight)
				{
					if (tetronimo[curTetronimo][pi] == L'X' && field[fi] != 0)
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}


int main()
{
	// List of 7 different tetronimo shapes
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");

	tetronimo[1].append(L"..X.");
	tetronimo[1].append(L".XX.");
	tetronimo[1].append(L".X..");
	tetronimo[1].append(L"...");

	tetronimo[2].append(L".X..");
	tetronimo[2].append(L".XX.");
	tetronimo[2].append(L"..X.");
	tetronimo[2].append(L"....");

	tetronimo[3].append(L"....");
	tetronimo[3].append(L".XX.");
	tetronimo[3].append(L".XX.");
	tetronimo[3].append(L"....");
			  
	tetronimo[4].append(L"..X.");
	tetronimo[4].append(L".XX.");
	tetronimo[4].append(L"..X.");
	tetronimo[4].append(L"....");

	tetronimo[5].append(L"....");
	tetronimo[5].append(L".XX.");
	tetronimo[5].append(L"..X.");
	tetronimo[5].append(L"..X.");

	tetronimo[6].append(L"....");
	tetronimo[6].append(L".XX.");
	tetronimo[6].append(L".X..");
	tetronimo[6].append(L".X..");


	// Create playing field
	field = new unsigned char[fieldWidth * fieldHeight];
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			// If at side or bottom of array, set to 9 for borders, otherwise set to 0
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}


	// Use command line as screen buffer
	wchar_t * screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++)
	{
		screen[i] = L' ';
	}
	HANDLE newConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(newConsole);
	DWORD bytesWritten = 0;


	// Game logic 	
	bool gameOver = false;
	int curPiece = 0;
	int curRotation = 0;
	int curX = fieldWidth / 2;
	int curY = 0;

	bool keyInput[4];
	bool rotateHold = false;

	int speed = 20;
	int speedCounter = 0;
	bool forceDown = false;
	int pieceCount = 0;
	int score = 0;

	vector<int> vLines;

	while (!gameOver)
	{
		// Game timing
		this_thread::sleep_for(50ms);
		speedCounter++;
		forceDown = (speedCounter == speed);		

		// Input
		for (int k = 0; k < 4; k++)
		{
			keyInput[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// Game logic
		curX += (keyInput[0] && DoesPieceFit(curPiece, curRotation, curX + 1, curY)) ? 1 : 0;
		curX -= (keyInput[1] && DoesPieceFit(curPiece, curRotation, curX - 1, curY)) ? 1 : 0;
		curY += (keyInput[2] && DoesPieceFit(curPiece, curRotation, curX, curY + 1)) ? 1 : 0;

		if (keyInput[3])
		{
			curRotation += (!rotateHold && DoesPieceFit(curPiece, curRotation + 1, curX, curY)) ? 1 : 0;
			rotateHold = true;
		}	
		else
		{
			rotateHold = false;
		}


		// Piece is being moved down
		if (forceDown)
		{
			if (DoesPieceFit(curPiece, curRotation, curX, curY + 1))
			{
				curY++;
			}
			else
			{
				// Lock piece
				for (int x = 0; x < 4; x++)
				{
					for (int y = 0; y < 4; y++)
					{
						if (tetronimo[curPiece][Rotate(x, y, curRotation)] == L'X')
						{
							field[(curY + y) * fieldWidth + (curX + x)] = curPiece + 1;
						}
					}
				}

				pieceCount++;
				if (pieceCount % 10 == 0 && speed >= 10)
				{
					speed--;
				}

				// Check for lines
				for (int y = 0; y < 4; y++)
				{
					if (curY + y < fieldHeight - 1)
					{
						bool line = true;
						for (int x = 1; x < fieldWidth - 1; x++)
						{
							line &= (field[(curY + y) * fieldWidth + x]) != 0;
						}

						if (line)
						{
							// Remove line and replace with =
							for (int x = 1; x < fieldWidth - 1; x++)
							{
								field[(curY + y) * fieldWidth + x] = 8;
							}

							vLines.push_back(curY + y);
						}
					}
				}

				score += 25;
				if (!vLines.empty())
				{
					score += (1 << vLines.size()) * 100;
				}

				//Choose next piece
				curX = fieldWidth / 2;
				curY = 0;
				curRotation = 0;
				curPiece = rand() % 7;

				// If piece doesn't fit
				gameOver = !DoesPieceFit(curPiece, curRotation, curX, curY);
			}

			speedCounter = 0;
		}
		

		//Draw field
		for (int x = 0; x < fieldWidth; x++)
		{
			for (int y = 0; y < fieldHeight; y++)
			{
				// Starts 2 in from borders, then uses ABCDEFG=# as 1-9 with array value as index
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[field[y * fieldWidth + x]];
			}
		}


		// Draw current piece
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				if (tetronimo[curPiece][Rotate(x, y, curRotation)] == L'X')
				{
					// +65 to get ASCII values
					screen[(curY + y + 2) * screenWidth + (curX + x + 2)] = curPiece + 65;
				}
			}
		}


		//Draw score
		swprintf_s(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE: %8d", score);


		if (!vLines.empty())
		{
			// Display frame
			WriteConsoleOutputCharacter(newConsole, screen, screenWidth * screenHeight, { 0, 0 }, &bytesWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : vLines)
			{
				for (int x = 1; x < fieldWidth - 1; x++)
				{
					for (int y = v; y > 0; y--)
					{
						field[y * fieldWidth + x] = field[(y - 1) * fieldWidth + x];
						field[x] = 0;
					}
				}
			}

			vLines.clear();
		}


		// Display frame
		WriteConsoleOutputCharacter(newConsole, screen, screenWidth * screenHeight, { 0, 0 }, &bytesWritten);		
	}

	// Oh dear
	CloseHandle(newConsole);
	cout << "Game Over! Score: " << score << endl;
	system("pause");

    return 0;
}

