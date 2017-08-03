#include <algorithm>
#include <fstream>
#include <sstream>
#include <ncurses/ncurses.h>
#include <ncurses/form.h>
#include "board.h"
#include "objects.h"
#include "settings.h"
#include <cstring>

using namespace std;

Settings* Settings::getInstance()
{
	static Settings *h = new Settings();
	return h;
}
int Settings::getAction() const
{
	init_pair(11, COLOR_YELLOW, COLOR_BLACK);
	int attr = COLOR_PAIR(11);
	werase(info_box);
	wattron(info_box, attr);
	wattron(info_box, A_BOLD);	
	mvwprintw(info_box,  1, 2, "Choose your action? \n  -push (ARROW)");
	wattroff(info_box, A_BOLD);
	mvwprintw(info_box,  2, 2, "* destroy (ENTER)");
	mvwprintw(info_box,  3, 2, "* push (ARROW)");
	wattroff(info_box, attr);

	wborder(info_box, 0, 0, 0, 0, 0, 0, 0, 0); 	
	touchwin(stdscr);
	wrefresh(info_box);
	int a = getch();
	werase(info_box);
	wrefresh(info_box);
	return a;
}

bool Settings::Init()
{
	ifstream board_file_stream;
	int M,N,T,K, ntreasures=0;
	string object_def; 
	Object *obj_tmp; 
	
	board_file_stream.open(board_path.c_str());
	if(!board_file_stream.is_open())
		return false;
	
	board_file_stream>>M>>N>>T;
	char **brd = new char*[M];
	Object ***objs = new Object**[M]; 
	brd[0] = new char[M*N];
	objs[0] = new Object*[M*N];

	for (int i=1;i<M;i++)
		{
			brd[i] = brd[0]+i*N;
			objs[i] = objs[0]+i*N;
		}


	getline(board_file_stream, object_def); 	
	for(int i=0;i<M;i++) 
		{
			for(int j=0;j<N;j++) 
				{
					board_file_stream>>brd[i][j];
					switch(brd[i][j])
						{
						case '.':
							brd[i][j] = CHAR_FLOOR;
							break;
						case '#':
							brd[i][j] = CHAR_WALL;
							break;
						case 'X':
							brd[i][j] = CHAR_EXIT;
							break;
						default:
							mvprintw(2,1,"Invalid character in board description (row = %d, column = %d)",i+1,j+1);
							board_file_stream.close();
							return false;
							break;
						}
					objs[i][j]=NULL;
				}
			getline(board_file_stream, object_def); 
		}
	board_file_stream>>K;
	getline(board_file_stream, object_def);

		  
	for(int i=0;i<K;i++)
		{
			getline(board_file_stream, object_def);	
			if((obj_tmp = CreateObject(object_def)) != NULL)
				{
					int a = obj_tmp->Row();
					int b = obj_tmp->Col();
					if(objs[a][b] == NULL && brd[a][b] != CHAR_WALL)
						objs[a][b] =obj_tmp;
					else
						{
							mvprintw(2,1,"Two objects in the same square of the board (row = %d, column = %d)",a+1,b+1);
							board_file_stream.close();
							return false;
						}
					if(obj_tmp->getType() == TREASURE)
						ntreasures++;
				}
			else
				{
					mvprintw(2,1,"Failed to load object %d (%s)", i+1, object_def.c_str());
					board_file_stream.close();
					return false;
						
				}
		}

	
	B->Init(M, N, T, brd, objs, ntreasures);
	B->nextActiveWorker();
	board_file_stream.close();
	return true;
}

Object* Settings::CreateObject(string object_def) const
{
	Object *res=NULL;
	string object_name, attr_name, attr_val;
	int row, column, dynamite, energy;
	bool contr=1; 
	object_def.erase(remove(object_def.begin(), object_def.end(), ' '), object_def.end());
	istringstream str(object_def);
	
	getline(str, object_name, ':'); 
  
	while(contr)
		{	
			getline(str, attr_name, '=');
			getline(str, attr_val, ',');
			if(attr_name[0]=='{') 
				attr_name.erase(attr_name.begin());
			if(attr_val[attr_val.length()-1]=='}') 
				{
					attr_val.erase(attr_val.length()-1,1);
					contr = 0; 
				}
     
			if(attr_name == "row")
				istringstream(attr_val) >> row;
			else if(attr_name == "column")
				istringstream(attr_val) >> column;
			else if(attr_name == "dynamite")
				istringstream(attr_val) >> dynamite;		
			else if(attr_name == "energy")
				istringstream(attr_val) >> energy;		
			else 
				return NULL;
		}

	if(object_name == "treasure")
		res = new Treasure(row-1, column-1);
	if(object_name == "crate")
		res = new Crate(row-1, column-1);
	if(object_name == "wheeled_crate")
		res = new WheeledCrate(row-1, column-1);
	if(object_name == "heavy_crate")
		res = new HeavyCrate(row-1, column-1);
	if(object_name == "fragile_crate")
		res = new FragileCrate(row-1, column-1);
	if(object_name == "worker")
		res = new Worker(row-1, column-1); 
	if(object_name == "sapper")
		res = new Sapper(row-1, column-1, dynamite); 
	if(object_name == "lifter")
		res = new Lifter(row-1, column-1); 
	if(object_name == "omni")
		res = new Omni(row-1, column-1, dynamite, energy); 
		
	if(res->isWorker()) 
		B->workers.push(dynamic_cast<Workers*>(res));
	return res;
} 
  
int Settings::getObjAttr(Object* obj) const
{
	int res = A_NORMAL;
	if(obj == NULL) 
		return res;
	res =  getObjAttr(obj->getType());
	if(obj == B->getActiveWorker())
		res = A_BOLD | res;
	return res;
}


int Settings::getObjAttr(int obj_type) const
{
	int res = A_NORMAL;
	switch(obj_type)
		{
		case TREASURE: 
			init_pair(TREASURE, COLOR_GREEN, COLOR_BLACK);
			res = COLOR_PAIR(TREASURE);
			break;
		case CRATE:
			init_pair(CRATE, COLOR_YELLOW, COLOR_BLACK);
			res = COLOR_PAIR(CRATE);	
			break;
		case WHEELED_CRATE:
			init_pair(WHEELED_CRATE, COLOR_MAGENTA, COLOR_BLACK);
			res = COLOR_PAIR(WHEELED_CRATE);		
			break;
		case HEAVY_CRATE:
			init_pair(HEAVY_CRATE, COLOR_RED, COLOR_BLACK);
			res = COLOR_PAIR(HEAVY_CRATE);		
			break;
		case FRAGILE_CRATE:
			init_pair(FRAGILE_CRATE, COLOR_CYAN, COLOR_BLACK);
			res = COLOR_PAIR(FRAGILE_CRATE);		
			break;
		case WORKER:
			init_pair(WORKER, COLOR_YELLOW, COLOR_BLACK);
			res = COLOR_PAIR(WORKER);
			break;
		case SAPPER:
			init_pair(SAPPER, COLOR_RED, COLOR_BLACK);
			res =  COLOR_PAIR(SAPPER);
			break;
		case LIFTER:
			init_pair(LIFTER, COLOR_CYAN, COLOR_BLACK);
			res =  COLOR_PAIR(LIFTER);
			break;
		case OMNI:
			init_pair(OMNI, COLOR_BLUE, COLOR_BLACK);
			res =  COLOR_PAIR(OMNI);
			break;

		} 
	return res; 

}
void Settings::DisplayBoard() 
{
	int x, y;
	x = scr_rows;
	y= scr_cols;
	corner_row = (x - B->Nrows())/2;
	corner_col = (y - B->Ncols())/2;
	
	x = corner_row; 
	for(int i=-1; i < B->Nrows()+1; i++)  
		{
			y = corner_col;
			if(i == -1 || i == B->Nrows()) 
				{
					for(int j=0; j < B->Ncols()+2; j++)  
						{
							mvaddch(x,y,CHAR_WALL);
							y++;
						}
				}	
			else 
				{
					mvaddch(x,y,CHAR_WALL);
					y++;
					for(int j=0; j < B->Ncols();j++)
						{
							if((B->board[i][j] == CHAR_FLOOR || B->board[i][j] == CHAR_EXIT) && B->objects[i][j] != NULL) 
								Put(B->objects[i][j],i,j);
							else 
								mvaddch(x,y,B->board[i][j]);
							y++;
						
						}
					mvaddch(x,y,CHAR_WALL);
					y++;
				}
			x++;	
		}
}

void Settings::DisplayInfo() const
{
	Workers *aw = B->getActiveWorker();
	const char *disp_energy, *disp_dynamite, *disp_name;
	int attr;

	disp_name = aw->Name();
	disp_energy = "inf";
		
	if(aw->getType() == OMNI)
		{
			ostringstream ss;
			ss << aw->getEnergy();
			disp_energy = ss.str().c_str();
		}

	werase(info_box);
	wrefresh(info_box);
	wborder(info_box, 0, 0, 0, 0, 0, 0, 0, 0);
	
	attr = getObjAttr(aw);
	wattron(info_box, attr);
	mvwprintw(info_box,  1, scr_cols/10 - strlen(disp_name)/2, disp_name);
	wattroff(info_box, attr);
	init_pair(12, COLOR_YELLOW,COLOR_BLACK);
	attr = COLOR_PAIR(12);
	wattron(info_box, attr);
	mvwprintw(info_box, 3, 2, "Row: %d", aw->Row()+1);
	mvwprintw(info_box, 4, 2, "Column: %d", aw->Col()+1);
	mvwprintw(info_box, 5, 2, "Dynamite: %d", aw->getDynamite());
	mvwprintw(info_box, 6, 2, "Energy: %s", disp_energy);
	mvwprintw(info_box, 11, 2, "Turn count / limit: %d / %d", B->getTurnCount(), B->getLimit());
	mvwprintw(info_box, 12, 2, "Remaining treasures: %d", B->ntreasures);
	wattroff(info_box, attr);
	wattron(info_box, attr | A_BOLD);
	mvwprintw(info_box, 15, 2, "* Switch active worker (TAB)");
	mvwprintw(info_box, 16, 2, "* Undo last move (u)");
	mvwprintw(info_box, 17, 2, "* Back to main menu (q)");
	wattroff(info_box, attr | A_BOLD);
	
	const char* crt_names[5] = {"Treasure", "Crate","Wheeled crate",  "Heavy crate", "Fragile crate"};

	int j=0;
	char tmp = CHAR_TREASURE;
	for(int i = TREASURE; i< FRAGILE_CRATE + 1; i++)
		{
			if(j>0)
				tmp = CHAR_CRATE;
			short int f,b;
			pair_content(i,&f,&b);
			if(f != COLOR_BLACK || b != COLOR_BLACK)
				{
					attr = COLOR_PAIR(i);
					wattron(info_box, attr);
					mvwprintw(info_box, 19+j, 2, "%c - %s", tmp, crt_names[j]);
					wattroff(info_box, attr);
					j++;
				}
		}
	touchwin(info_box);
	wrefresh(info_box);
}

void Settings::Put(Object *obj, int dest_row, int dest_col) const
{
	int attr=0;
	char symbol;

	if(obj != NULL)
		symbol = obj->getChar();
	else
		symbol = B->board[dest_row][dest_col];

	attr = getObjAttr(obj);

	if(B->board[dest_row][dest_col] == CHAR_EXIT && obj != NULL)
		attr = A_BLINK | attr;

	attron(attr);
	mvaddch(corner_row + dest_row + 1, corner_col + dest_col + 1, symbol);
	attroff(attr);
}

void Settings::Move(Object *obj, int dest_row, int dest_col) const
{
	Put(NULL, obj->Row(), obj->Col());
	Put(obj, dest_row, dest_col);
}

void Settings::Remove(Object* obj) const
{
	Put(NULL, obj->Row(), obj->Col());
}

void Settings::Click(int row, int col) const
{
	int i,j;
	i = row - corner_row - 1;
	j = col - corner_col - 1;
	if(i>=0 && j>=0 && i < B->Nrows() && j < B->Ncols())
		if(B->objects[i][j] != NULL)
			if(B->objects[i][j]->isWorker())
				{
					Workers *aw = B->getActiveWorker();
					B->setActiveWorker(dynamic_cast<Workers*>(B->objects[i][j]));					
					DisplayInfo();
					Put(aw, aw->Row(), aw->Col());
					Put(B->objects[i][j], i, j);
					refresh();
				}
}

void Settings::nextActiveWorker() const
{
	Workers *aw = B->getActiveWorker();
	B->nextActiveWorker();
	switchActiveWorker(aw);
	DisplayInfo();
}

void Settings::switchActiveWorker(Workers* old_aw) const
{ 
	Workers *aw = B->getActiveWorker();
	Put(old_aw, old_aw->Row(), old_aw->Col());
	Put(aw, aw->Row(), aw->Col());

}

bool Settings::WelcomeScreen()  
{
	int row = 3;
	int col = scr_cols/2 - 25;
	init_pair(11, COLOR_YELLOW,COLOR_BLACK);
	int attr = COLOR_PAIR(11) | A_BOLD;
	WINDOW *form_window;
	bool res = false;
	char c;	

	clear();
	wborder(stdscr, 0, 0, 0, 0, 0, 0, 0, 0);

	attron(attr);	
	mvprintw(row,col," _____       _         _                            ");
	mvprintw(row+1,col,"/  ___|     | |       | |                 _     _   ");
	mvprintw(row+2,col,"\\ `--.  ___ | | _____ | |__   __ _ _ __ _| |_ _| |_ ");
	mvprintw(row+3,col," `--. \\/ _ \\| |/ / _ \\| '_ \\ / _` | '_ \\_   _|_   _|");
	mvprintw(row+4,col,"/\\__/ / (_) |   < (_) | |_) | (_| | | | ||_|   |_|  ");
	mvprintw(row+5,col,"\\____/ \\___/|_|\\_\\___/|_.__/ \\__,_|_| |_| ");
	attroff(attr);                                                          
	
	form_window = subwin(stdscr, 4, 60, row+13, scr_cols/2 - 30);
	
	noecho();

	bff_field[0] = new_field(1, 30, 5, 18, 0, 0);
	bff_field[1] = NULL;
	keypad(form_window,1);
	
	set_field_back(bff_field[0], A_UNDERLINE);  
	field_opts_off(bff_field[0], O_AUTOSKIP);   
	
	board_file_form = new_form(bff_field);
	set_form_sub(board_file_form, form_window);

	attr = COLOR_PAIR(11);
	
	attron(attr);
	
	mvprintw(row+8, col+12, "* Start game (ENTER)");
	mvprintw(row+9, col+12, "* Set board path (TAB)");
	mvprintw(row+10, col+12, "* Quit (%c)", KEY_QUIT);
	attroff(attr);
	refresh();
			
	
	while((c = wgetch(form_window)))
		{
			if(c == KEY_QUIT)
				{
					res = true;
					break;
				}
			if(c == KEY_DETONATE)
				break;
				
			if(c == KEY_TAB)
				{
					box(form_window, 0, 0);
					curs_set(1);
					post_form(board_file_form);
					wattron(form_window, attr);
					mvwprintw(form_window, 1, 1, "Board file path:");
					wattroff(form_window, attr);
					wattron(form_window, attr | A_BOLD);
					
					mvwprintw(form_window, 2, 17, "Press ENTER to accept ...");
					wattroff(form_window, attr | A_BOLD);

					mvwprintw(form_window, 1, 17, board_path.c_str());				
					touchwin(form_window);
					wrefresh(form_window);
					while((c = wgetch(form_window)) != KEY_DETONATE)
						{			

							int x, y;
							getyx(form_window, y, x);
							if(c == KEY_BACKSPACE_ && board_path.size() > 0)
								{
									board_path.resize(board_path.size()-1);
									wmove(form_window,  y, x -1);
									waddch(form_window, ' ');				
								}			
							else
								board_path = board_path + c;
							mvwprintw(form_window, 1, 17, board_path.c_str());	
							touchwin(form_window);
							wrefresh(form_window);
						}
					unpost_form(board_file_form);
					wclear(form_window);
					curs_set(0);
				}
		}	

	free_form(board_file_form);
	free_field(bff_field[0]);
	delwin(form_window);
	erase();                                                                      
	wborder(stdscr, 0, 0, 0, 0, 0, 0, 0, 0);
	return res;
	
}

bool Settings::GameOverScreen(game_status game_result) const
{
	int row = 3;
	int col = 3*scr_cols/5 - 25;
	int attr;
	char c;
	if(game_result == GS_INPROGRESS)
		return false;

	switch(game_result)
		{

		case GS_WON:
			
			init_pair(11, COLOR_GREEN, COLOR_BLACK);
			attr = COLOR_PAIR(11) | A_BOLD | A_BLINK;
   
			attron(attr);	

			mvprintw(row,col,"__   __                     _       _ ");
			mvprintw(row+1,col,"\\ \\ / /                    (_)     | |");
			mvprintw(row+2,col," \\ V /___  _   _  __      ___ _ __ | |");
			mvprintw(row+3,col,"  \\ // _ \\| | | | \\ \\ /\\ / / | '_ \\| |");
			mvprintw(row+4,col,"  | | (_) | |_| |  \\ V  V /| | | | |_|");
			mvprintw(row+5,col,"  \\_/\\___/ \\__,_|   \\_/\\_/ |_|_| |_(_)");
			attroff(attr);                                                          
			break;	  
		case GS_LOST:

			init_pair(11, COLOR_RED, COLOR_BLACK);
			attr = COLOR_PAIR(11) | A_BOLD | A_BLINK;
			attron(attr);
			mvprintw(row,col,"__   __            _                _ ");
			mvprintw(row+1,col,"\\ \\ / /           | |              | |");
			mvprintw(row+2,col," \\ V /___  _   _  | | ___  ___  ___| |");
			mvprintw(row+3,col,"  \\ // _ \\| | | | | |/ _ \\/ __|/ _ \\ |");
			mvprintw(row+4,col,"  | | (_) | |_| | | | (_) \\__ \\  __/_|");
			mvprintw(row+5,col,"  \\_/\\___/ \\__,_| |_|\\___/|___/\\___(_)");
			attroff(attr);                  
                                     
			break;
		}
	init_pair(12, COLOR_YELLOW, COLOR_BLACK);
	attr = COLOR_PAIR(12) | A_BOLD;
	wattron(info_box, attr);	
	mvwprintw(info_box, 25, 2,"Press ENTER ...");
	wattroff(info_box, attr);                                                          
	touchwin(info_box);
	wrefresh(info_box);
	while(c = getch())
		{
			if(c == KEY_QUIT)
				return true;
			if(c == KEY_DETONATE)
				return false;
		}	
		 		  
}

Settings::Settings(){ 
	keypad(initscr(),1);
	use_default_colors();
	curs_set(0);
	noecho(); 
	mousemask(BUTTON1_CLICKED, NULL);
	start_color();			
	wborder(stdscr, 0, 0, 0, 0, 0, 0, 0, 0);
	getmaxyx(stdscr,scr_rows,scr_cols);
	info_box = subwin(stdscr, scr_rows-2, scr_cols/5, 1, 2);
	board_path = "boards/board1";
	B = Board::getInstance();
}

Settings::~Settings()
{
	delwin(info_box);
	endwin();
}
