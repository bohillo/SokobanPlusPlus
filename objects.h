#ifndef OBJECTS_H
#define OBJECTS_H

#include <stack>


const char CHAR_FLOOR = ' '; 
const char CHAR_WALL = '#'; 
const char CHAR_EXIT = 'X';
const char CHAR_WORKER = '@';
const char CHAR_TREASURE = '$';
const char CHAR_CRATE = '#';

enum object_type {TREASURE = 1, CRATE, WHEELED_CRATE, HEAVY_CRATE, FRAGILE_CRATE, WORKER, SAPPER, LIFTER, OMNI};

class Board;

class Object
{
	Object(const Object&);
	void operator=(const Object&);
protected:
	int row;
	int col;
	int dest_row;
	int dest_col;
	std::stack<int> prev_rows;
	std::stack<int> prev_cols;
	Board* B;

	void InitCoordinates(int, int);
	void Move();
	virtual void setDestination(int);
public:
	int Row() const;
	int Col() const ;
	virtual bool Move(int);
	virtual void Undo();
	virtual object_type getType() const=0;
	virtual char getChar() const=0;
	virtual const char* Name() const=0;
	virtual bool isDestructible() const=0;
	virtual bool isCrate() const=0;
	virtual bool isWorker() const=0;
	Object(int, int);
};

class Crates: public Object
{
	bool destroyed;
public:
	virtual	object_type getType() const=0;
	bool isDestroyed() const;
	virtual bool isDestructible() const;
	virtual char getChar() const;
	virtual	bool Destroy();
	virtual	bool Push(int);
	bool isCrate() const;
	bool isWorker() const;
	void Restore();
	Crates(int, int);

};

class Treasure: public Crates 
{
public:
	object_type getType() const;
	const char* Name() const;
	char getChar() const;
	 bool isDestructible() const;
	bool Push(int);
	bool Destroy();
	Treasure(int, int);
};


class Crate: public Crates
{
public:
	object_type getType() const;
	const char* Name() const;
	Crate(int, int);
};


class WheeledCrate: public Crates
{
	void setDestination(int);
public:
	object_type getType() const;
	const char* Name() const;
	WheeledCrate(int, int);
};

class HeavyCrate: public Crates
{
public:
	object_type getType() const;
	const char* Name() const;
	bool Push(int);
	HeavyCrate(int, int);
};

class FragileCrate: public Crates
{
public:
	object_type getType() const;
	const char* Name() const;
	FragileCrate(int, int);
};


class Workers: public Object
{
protected:
	int dynamite;
	int energy; 
	std::stack<Crates*> moved_crates; 

	virtual Crates* isLegal() const;
	virtual bool Action(Crates*, int);
	virtual void UndoAction();

public:
	
	virtual	object_type getType() const=0;
	char getChar() const;
	int getDynamite() const;
	int getEnergy() const;
	bool isDestructible() const;
	bool isCrate() const;
	bool isWorker() const;
	void Undo();
	bool Move(int);
	Workers(int,int);
};

class Worker: public Workers 
{
	Crates* isLegal() const;
public:
	object_type getType() const;
	const char* Name() const;
	Worker(int,int);
};

class Sapper: public Workers
{
	Crates* isLegal() const;
	bool Action(Crates*, int);
	void UndoAction();

public:
	object_type getType() const;
	const char* Name() const;
	Sapper(int, int, int);

};

class Lifter: public Workers
{
	Crates* isLegal() const;
public:
	object_type getType() const;
	const char* Name() const;
	Lifter(int, int);
};

class Omni: public Workers
{
	bool Action(Crates*, int);
	void UndoAction();	
public:
	object_type getType() const;
	const char* Name() const;
	Omni(int, int, int, int);
};
#endif
