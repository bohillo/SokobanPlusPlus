#include <string>
#include <ncurses/ncurses.h>
#include "board.h"
#include "objects.h"
#include "settings.h"
using namespace std;

void Object::setDestination(int c)
{
	switch(c) 
		{
		case KEY_UP:
			dest_row--;
			break;
		
		case KEY_DOWN:
			dest_row++;
			break;

		case KEY_LEFT:
			dest_col--;
			break;

		case KEY_RIGHT:
			dest_col++;
			break;	
		}

}

void Object::Undo()
{
	dest_row = prev_rows.top();
	dest_col = prev_cols.top();
	Move();
	prev_rows.pop();
	prev_cols.pop();

}

void Object::InitCoordinates(int row_, int col_)
{
	row = row_;
	col = col_;
	dest_row = row;
	dest_col = col;
}

int Object::Row() const
{
	return row;
}


int Object::Col() const
{
	return col;
}


void Object::Move()
{
	Settings *S = Settings::getInstance();
	S->Move(this, dest_row, dest_col); 
	B->Put(this, dest_row, dest_col);
	InitCoordinates(dest_row, dest_col);
}

bool Object::Move(int c)
{
  	dest_col = col;
	dest_row = row;
	setDestination(c);

	if(B->isFree(dest_row, dest_col))
		{	
			prev_rows.push(row);
			prev_cols.push(col);
			Move();
			return true;
		}
	return false;
}

Object::Object(int row_, int col_)
{
	InitCoordinates(row_, col_);
	B = Board::getInstance();
}


/* CRATES */

bool Crates::Destroy()
{
	Settings *S = Settings::getInstance();
	destroyed = true;
	S->Remove(this);
	B->Remove(this);
	return true;
}

bool Crates::Push(int c)
{
	return Object::Move(c);
}

bool Crates::isDestroyed() const
{
	return destroyed;
}

bool Crates::isDestructible() const
{
	return true;
}


bool Crates::isCrate() const
{
	return true;
}


bool Crates::isWorker() const
{
	return false;
}

void Crates::Restore()
{
	destroyed = false;
}

char Crates::getChar() const
{
	return CHAR_CRATE;
}


Crates::Crates(int row_, int col_) : destroyed(false), Object(row_, col_) {}

/* TREASURE */

bool Treasure::Destroy()
{
	return false;
}


bool Treasure::isDestructible() const
{
	return false;
}

bool Treasure::Push(int c)
{
	if(Crates::Push(c))
		{
			if(B->isExit(row, col))
				Crates::Destroy();
			return true;
		}
	return false;
}

object_type Treasure::getType() const 
{
	return TREASURE;
}


char Treasure::getChar() const
{
	return CHAR_TREASURE;
}

const char* Treasure::Name() const
{
	return "Treasure";
}


Treasure::Treasure(int row_, int col_) : Crates(row_, col_) {}


/* CRATE */

object_type Crate::getType() const 
{
	return CRATE;
}

const char* Crate::Name() const
{
	return "Crate";
}

Crate::Crate(int row_, int col_) : Crates(row_, col_) {}

void WheeledCrate::setDestination(int c)
{
	int final_row, final_col;
	do 
		{
			final_row = dest_row;
			final_col = dest_col;
			Object::setDestination(c);
		}	
	while(B->isFree(dest_row, dest_col));

	if(!B->isWorker(dest_row, dest_col))
		{
			dest_row = final_row;
			dest_col = final_col;
		}
	else
		{
			dest_row = row;
			dest_col = col;
		}
}


object_type WheeledCrate::getType() const 
{
	return WHEELED_CRATE;
}

const char* WheeledCrate::Name() const
{
	return "WheeledCrate";
}

WheeledCrate::WheeledCrate(int row_, int col_) : Crates(row_, col_) {}


object_type HeavyCrate::getType() const 
{
	return HEAVY_CRATE;
}


bool HeavyCrate::Push(int c)
{
	return 0;
}


const char* HeavyCrate::Name() const
{
	return "HeavyCrate";
}

HeavyCrate::HeavyCrate(int row_, int col_) : Crates(row_, col_) {}


object_type FragileCrate::getType() const 
{
	return FRAGILE_CRATE;
}


const char* FragileCrate::Name() const
{
	return "FragileCrate";
}



FragileCrate::FragileCrate(int row_, int col_) : Crates(row_, col_) {}


/*WORKERS */

bool Workers::isDestructible() const
{
	return false;
}

bool Workers::isCrate() const
{
	return false;
}

bool Workers::isWorker() const
{
	return true;
}


Crates* Workers::isLegal() const
{
	Crates *res = B->isCrate(dest_row, dest_col);
	if(res != NULL)
		return res;
	return NULL;

}

bool Workers::Action(Crates* crt, int c)
{
	if(crt->Push(c))
		return Object::Move(c);
}

bool Workers::Move(int c)
{
	Crates *crt = NULL;
	bool res = false;

	if(energy != 0) 
		{
			res = Object::Move(c);
			if(!res && (crt = isLegal()) != NULL) 
				res = Action(crt, c);
			if(res) 
				{
					energy--;
					B->incrTurnCount();
					B->addActiveWorker(this);
					moved_crates.push(crt);
				}	
		}
	return res;
}


void Workers::Undo()
{

	energy++;
	Object::Undo();
	
	if(moved_crates.top() != NULL)
		UndoAction();

	moved_crates.pop();
}

void Workers::UndoAction()
{
	if(moved_crates.top()->isDestroyed())
		{
			B->RestoreDeletedCrate();
			moved_crates.top()->Restore();
		}	
moved_crates.top()->Undo();
}

int Workers::getEnergy() const
{
	return energy;
}

int Workers::getDynamite() const
{
	return dynamite;
}


char Workers::getChar() const
{
	return CHAR_WORKER;
}

Workers::Workers(int row_, int col_) : energy(-1), dynamite(0), Object(row_, col_){}

/* WORKER */

Crates* Worker::isLegal() const 
{
	Crates *res = Workers::isLegal();
	if(res != NULL)
		if(res->getType() != FRAGILE_CRATE)
			return res;
	return NULL;
}

object_type Worker::getType() const
{
	return WORKER;
}

const char* Worker::Name() const 
{
	return "Worker";
}

Worker::Worker(int row_, int col_) : Workers(row_, col_) {}


/* SAPPER */
object_type Sapper::getType() const
{
	return SAPPER;
}


const char* Sapper::Name() const 
{
	return "Sapper";
}


Crates* Sapper::isLegal() const 
{
	Crates *res = B->isDestructible(dest_row, dest_col);
	if(res != NULL)
		return res;
	return NULL;
}

bool Sapper::Action(Crates* crt, int c)
{
	if(dynamite > 0)
		{
			prev_rows.push(row);
			prev_cols.push(col);
			InitCoordinates(row, col);
			dynamite--;
			return crt->Destroy();
		}
	return false; 
}


void Sapper::UndoAction()
{
	Crates *crt = moved_crates.top();
	B->RestoreDeletedCrate();
	crt->Restore();
	dynamite++;
}


Sapper::Sapper(int row_, int col_, int dynamite_) : Workers(row_, col_) 
{
	dynamite = dynamite_;
}


/* LIFTER */


Crates* Lifter::isLegal() const 
{
	Crates *res = B->isFragile(dest_row, dest_col);
	if(res != NULL)
		return res;
	return NULL;
}


object_type Lifter::getType() const
{
	return LIFTER;
}


const char* Lifter::Name() const 
{
	return "Lifter";
}

Lifter::Lifter(int row_, int col_) : Workers(row_, col_) {}


/* OMNI */

bool Omni::Action(Crates* crt, int c)
{
	if(energy == 0)
		return false;
	int type = crt->getType();
	int action_key;
	bool destroy = (dynamite > 0) && (type!=TREASURE) ;
	Settings *S = Settings::getInstance();
	
	if(destroy)
		{
			action_key = S->getAction();
			if(action_key == KEY_DETONATE)
				{
					prev_rows.push(row);
					prev_cols.push(col);
					InitCoordinates(row, col);
					dynamite--;
					return crt->Destroy();
				}
			else if(action_key == c) 
				return Workers::Action(crt,c);
			else 
				return false;		
		}
	else
		return Workers::Action(crt,c);
		
}


void Omni::UndoAction()
{
	Crates *crt = moved_crates.top();
	if(crt->isDestroyed())
		{
			B->RestoreDeletedCrate();
			crt->Restore();
			dynamite++;
		}
	else
		Workers::UndoAction();
}

object_type Omni::getType() const
{
	return OMNI;
}


const char* Omni::Name() const 
{
	return "Omni";
}

Omni::Omni(int row_, int col_, int dynamite_, int energy_) : Workers(row_, col_) 
{
	dynamite = dynamite_;
	energy = energy_;
}

