#ifndef BOARD_H
#define BOARD_H


#include <queue>
#include <stack>


enum game_status {GS_INPROGRESS, GS_LOST, GS_WON};

class Object;
class Workers;
class Crates;

class Board { 
	friend class Settings;
	int nrows;
	int ncols;
	int limit;
	int turn_count;
	int ntreasures;

	std::queue<Workers*> workers;
	std::stack<Crates*> deleted_crates;
	std::stack<Workers*> active_workers;

	char **board;  
	Object ***objects; 
	Workers *active_worker;

	
	bool isOnBoard(int, int) const;
	void operator=(const Board&); 
	Board(const Board&); 
	Board();
public:
	int Nrows() const;
	int Ncols() const;
	int getLimit() const;
	int getTurnCount() const;
	game_status GameStatus() const;
	Workers* getActiveWorker() const;

	bool isExit(int, int) const;
	bool isWorker(int, int) const;
	bool isFree(int,int) const; 
	Crates* isDestructible(int, int) const;
	Crates* isCrate(int, int) const; 
	Crates* isFragile(int, int) const;
	
	void Init(int, int, int, char**, Object***, int);
	bool Put(Object*, int, int);
	void Remove(Crates*);
	void setActiveWorker(Workers*);
	void nextActiveWorker();
	void Destroy();
	void incrTurnCount();
	void addActiveWorker(Workers*);
	void Undo();
	void RestoreDeletedCrate();

	static Board* getInstance();
	~Board();
};

#endif
