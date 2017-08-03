#ifndef SETTINGS_H

#define SETTINGS_H

#include <string>
#include <ncurses/form.h>

const char KEY_DETONATE = '\n';
const char KEY_UNDO_ = 'u';
const char KEY_QUIT = 'q';
const char KEY_TAB = 9;
const char KEY_BACKSPACE_ = 127;

class Board;
class Object;
class Workers;
enum game_status;

class Settings 
{
	int corner_row;
	int corner_col;
	int scr_rows;
	int scr_cols;
	std::string board_path;
	FIELD *bff_field[2];
    FORM  *board_file_form;
	WINDOW *info_box;
	Board* B;

	Object* CreateObject(std::string) const;
	void Put(Object*, int, int) const;
	int getObjAttr(Object*) const; 
	int getObjAttr(int) const; 
	void operator=(const Settings&);
	Settings(const Settings&);
	Settings();

public:
	int CornerRow() const;
	int CornerCol() const;
	int getAction() const;
	void Remove(Object*) const;
	void DisplayInfo() const;
	void Move(Object*, int, int) const;
	void switchActiveWorker(Workers*) const;
	void nextActiveWorker() const;
	void Click(int, int) const;
	bool GameOverScreen(game_status) const;

	bool Init();
	void DisplayBoard();
	bool WelcomeScreen();

	static Settings* getInstance();
	~Settings();
};
 

#endif
