#include "board.h"
#include "objects.h"
#include "settings.h"
using namespace std;


void Board::Init(int M, int N, int T, char** brd, Object*** objs, int ntreasures_)
{
	nrows = M;
	ncols = N;
	limit = T;
	board = brd;
	objects = objs;
	turn_count = 0;
	ntreasures = ntreasures_;
}

bool Board::Put(Object *obj, int dest_row, int dest_col) 
{
	if(isFree(dest_row, dest_col))
		{
			objects[obj->Row()][obj->Col()] = NULL;
			objects[dest_row][dest_col] = obj;
			return true;
		}
	return false;
}

void Board::Remove(Crates *obj)
{
	int i = obj->Row();
	int j = obj->Col();
	deleted_crates.push(obj);
	objects[i][j] = NULL;
	if(obj->getType() == TREASURE)
	   ntreasures--;
}

Board::Board()
{
	Init(0, 0, 0, NULL, NULL, 0);	
}

Board* Board::getInstance()
{
	static Board *g = new Board();
	return g;
}

int Board::Ncols() const
{
	return ncols;
}


int Board::Nrows() const
{
	return nrows;
}

int Board::getLimit() const 
{
	return limit;
}


int Board::getTurnCount() const 
{
	return turn_count;
}

void Board::setActiveWorker(Workers *W)
{
	active_worker = W;
}

Workers* Board::getActiveWorker() const
{
	return active_worker;
}

void Board::nextActiveWorker() 
{
	Workers *w;
	w = workers.front();
	workers.pop();
	workers.push(w);
	setActiveWorker(workers.front());
}

bool Board::isOnBoard(int row, int col) const
{
	if(0 <= row  &&  0 <= col &&  nrows > row &&  ncols > col)
		return true;
	return false;
}

bool Board::isFree(int row, int col) const
{
	return isOnBoard(row, col) && (board[row][col] == CHAR_FLOOR || board[row][col] == CHAR_EXIT) && objects[row][col] == NULL;
}

Crates* Board::isDestructible(int row, int col) const
{
	if(isOnBoard(row,col) && objects[row][col] != NULL) 
		if(objects[row][col]->isDestructible())
				return dynamic_cast<Crates*>(objects[row][col]);
	return NULL;
}


bool Board::isWorker(int row, int col) const
{
	if(isOnBoard(row, col) && objects[row][col] != NULL)
		return objects[row][col]->isWorker();
	return false;
}

Crates* Board::isCrate(int row, int col) const
{
	if(isOnBoard(row, col) &&  objects[row][col] != NULL)
		if(objects[row][col]->isCrate())
			return dynamic_cast<Crates*>(objects[row][col]);
	return NULL;
} 

Crates* Board::isFragile(int row, int col) const
{
	if(isOnBoard(row, col) &&  objects[row][col] != NULL)
		if(objects[row][col]->getType() == FRAGILE_CRATE)
			return dynamic_cast<Crates*>(objects[row][col]); 
	return NULL;
}

bool Board::isExit(int row, int col) const
{
	return (board[row][col] == CHAR_EXIT);
}

game_status Board::GameStatus() const
{
	if(ntreasures == 0)
		return GS_WON;
	if(turn_count >= limit)
		return GS_LOST;

	return GS_INPROGRESS;
}

void Board::incrTurnCount() 
{
	turn_count++;
}

void Board::addActiveWorker(Workers* W)
{
	active_workers.push(W);
}

void Board::Destroy()
{

	while(!deleted_crates.empty())
		{
			delete deleted_crates.top();
			deleted_crates.pop();
		}

	while(!workers.empty())
		workers.pop();

	while(!active_workers.empty())
		active_workers.pop();

	active_worker = NULL;
	if(objects != NULL && board != NULL) {
		for(int i=0;i<nrows;i++)
			for(int j=0;j<ncols;j++) 
				{
					delete objects[i][j];
					objects[i][j] = NULL;
				}
	
		delete[] board[0];
		delete[] objects[0];
		board[0] = NULL;
		objects[0] = NULL;
		delete[] objects;
		delete[] board;

	}
	Init(0, 0, 0, NULL, NULL, 0);

}

void Board::Undo()
{

	if(active_workers.empty())
		return;
	Workers *old_aw = getActiveWorker();
	setActiveWorker(active_workers.top());
	Settings::getInstance()->switchActiveWorker(old_aw);
	active_workers.top()->Undo();
	active_workers.pop();
	turn_count--;
}

void Board::RestoreDeletedCrate()
{
	if(deleted_crates.empty())
		return;
	Crates *crt = deleted_crates.top();
	objects[crt->Row()][crt->Col()] = crt;
	if(crt->getType() == TREASURE)
		ntreasures++;
	Settings::getInstance()->Move(crt, crt->Row(), crt->Col());
	deleted_crates.pop();
}

 
Board::~Board()
{
	Destroy();
}
