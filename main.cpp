#include <ncurses/ncurses.h>
#include "board.h"
#include "objects.h"
#include "settings.h"
using namespace std;
 
int main(int argc, char *argv[])
{

	Board *B = Board::getInstance();

	
	Settings* S = Settings::getInstance();
	int c;
	game_status gs;
	MEVENT mouse_input; 
	
	while(!S->WelcomeScreen())
		{
			if(!S->Init()) 
				{
					mvprintw(1,1, "Error in loading board file. Press any key ...");
					B->Destroy();
					getch();
					continue;
				}

			S->DisplayBoard();
			S->DisplayInfo();
			while((gs = B->GameStatus()) == GS_INPROGRESS)
				{		
					c = getch();
					if(c == KEY_QUIT)
						break;
					else if(c == KEY_TAB)
						S->nextActiveWorker();
					else if(c == KEY_MOUSE)
						{ 		
							getmouse(&mouse_input);
							S->Click(mouse_input.y, mouse_input.x);
						}	
					else if(c == KEY_UNDO_)
						B->Undo();
					else
						B->getActiveWorker()->Move(c);					
 		
					S->DisplayInfo();			   
				}	

		
			S->GameOverScreen(gs);

			B->Destroy();
			clear();
		}
	
	delete B;
	delete S;
	
	return 0;
}
