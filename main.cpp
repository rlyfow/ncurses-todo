#include <ncurses.h>
#include <string>
#include <fstream>
using namespace std;

struct entry
{
	string name;
	string desc;
	bool done;
};

struct state
{
	entry * new_entry;
	int entry_count;
	entry * tasks;
	int cursor;
	int maxy;
	int maxx;
	char input;
	int screen;
	int scroll;
};

int screenSetup();
state * appSetup();
int handleInput(state * app);
int drawWindow(state * app);
int addEntryScreen(state * app);
string wrapString(string message, int len);
int addEntry(state * app);
int addDescScreen(state * app);
int fitPhraseToWindow(string message, int x, int y, int maxy, int maxx);
int saveState(state * app);

int main()
{
	screenSetup();
	
	state * app = appSetup();
	
	while(true)
	{
		clear();
		refresh();
		getmaxyx(stdscr, app->maxy, app->maxx);
		
		if(app->screen == 1)
		{
			addEntryScreen(app);
		}
		
		if(app->screen == 2)
		{
			addDescScreen(app);
		}
		
		if(app->screen == 0)
		{
			drawWindow(app);
			move(app->cursor-app->scroll, 0);
			app->input = getch();
			
			handleInput(app);
		}
	}
	
	delete app;
	
	endwin();
	return 0;
}

int deleteEntry(state * app, int id)
{
	app->entry_count--;
	
	entry * new_list = new entry[app->entry_count];
	
	if(app->entry_count == 0)
	{
		delete[] new_list;
		return 0;
	}
	
	int j=0;
	for(int i=0; i<app->entry_count; i++)
	{
		if(i==id) continue;
		new_list[j] = app->tasks[i];
		j++;
	}
	
	if(app->cursor>=app->entry_count) app->cursor--;
	
	delete[] app->tasks;
	app->tasks = new_list;
	
	return 0;
}

int saveState(state * app)
{
	fstream file;
	file.open("todo_save.txt", ios::out | ios::trunc);
	file << app->entry_count;
	
	for(int i=0; i<app->entry_count; i++)
	{
		file << "\n" << app->tasks[i].name << "\n" << app->tasks[i].desc << "\n" << app->tasks[i].done;
	}
	
	return 0;
}

int fitPhraseToWindow(string message, int x, int y, int maxy, int maxx)
{
	message+= " ";
	int word_start = 0;
	int word_end = 0;
	int word_len = 0;
	
	int max_space = maxx-x-1;
	int space_left = max_space;
	
	move(y, x);
	
	while(word_end <= (int)message.length())
	{
		if(y >= maxy) break;
		if(message[word_end] == ' ')
		{
			if(word_len+1 > space_left)
			{
				y++;
				move(y,x);
				space_left = max_space;
			}
			
			printw("%s ", message.substr(word_start, word_len).c_str());
			space_left-=word_len+1;
			
			word_end++;
			word_start=word_end;
			
			
			word_len = 0;
		}
		else
		{
			word_end++;
			word_len++;
			if(word_len > max_space)
			{
				printw("%s ", wrapString(message.substr(word_start, word_len), max_space).c_str());
				
				break;
			}
		}
	}
	
	return y;
}

int addEntry(state * app)
{
	entry * new_list = new entry[app->entry_count];
	
	//rewrite previous entries to new
	for(int i=0; i<app->entry_count-1; i++)
	{
		new_list[i] = app->tasks[i];
	}
	
	delete[] app->tasks;
	app->tasks = new_list;
	
	app->tasks[app->entry_count-1] = *app->new_entry;
	
	delete app->new_entry;
	app->new_entry = nullptr;
	
	return 0;
}

int addDescScreen(state * app)
{
	move(0,0);
	printw("Title:");
	move(1,0);
	int y_name = fitPhraseToWindow(app->new_entry->name, 0, 1, app->maxy, app->maxx);
	move(y_name+1,0);
	printw("Description:");
	fitPhraseToWindow(app->new_entry->desc, 0, y_name+2, app->maxy, app->maxx);
	
	app->input = getch();
	if(app->input == 10) 
	{
		app->screen = 0; //enter key
		app->entry_count++;
		addEntry(app);
		saveState(app);
		clear();
	}
	if(app->input == 27) 
	{
		app->screen = 0; //escape key
		delete app->new_entry;
		app->new_entry = nullptr;
		clear();
	}
	else if(app->input >= 32 && app->input < 127) app->new_entry->desc += app->input;
	else if(app->input == 127) app->new_entry->desc = app->new_entry->desc.substr(0, app->new_entry->desc.size()-1);
	
	return 0;
}

int addEntryScreen(state * app)
{
	if(app->new_entry == nullptr)
	{
		app->new_entry = new entry;
		app->new_entry->done = false;
	}
	
	move(0,0);
	printw("Title:");
	move(1,0);
	fitPhraseToWindow(app->new_entry->name, 0, 1, app->maxy, app->maxx);
	
	app->input = getch();
	if(app->input == 10) 
	{
		app->screen = 2; //enter key
		clear();
	}
	if(app->input == 27) 
	{
		app->screen = 0; //escape key
		delete app->new_entry;
		app->new_entry = nullptr;
		clear();
	}
	else if(app->input >= 32 && app->input < 127) app->new_entry->name += app->input;
	else if(app->input == 127) app->new_entry->name = app->new_entry->name.substr(0, app->new_entry->name.size()-1);
	
	return 0;
}

string wrapString(string message, int len)
{
	if((int)message.length() > len)
	{
		message = message.substr(0, len);
	}
	
	return message;
}

int drawWindow(state * app)
{
	while(app->cursor < app->scroll) app->scroll--;
	while(app->cursor-app->scroll >= app->maxy-3) app->scroll++;
	
	if(app->maxy-3 > 0 && app->entry_count > 0)
	{
		for(int i=app->scroll; i<app->entry_count; i++)
		{
			if(i-app->scroll > app->maxy-3) break;
			if(app->tasks[i].done == true)
			{
				attron(A_DIM);
				move(i-app->scroll, 0);
				printw("X");
			}
			move(i-app->scroll, 1);
			if(i == app->cursor) attron(A_REVERSE);
			printw("%s", wrapString(app->tasks[i].name, app->maxx/2).c_str());
			attroff(A_REVERSE);
			attroff(A_DIM);
			
			
		}
		move(0, app->maxx/2+1);
		attron(A_REVERSE);
		printw("Title:");
		attroff(A_REVERSE);
		move(1, app->maxx/2+1);
		
		int y_name = fitPhraseToWindow(app->tasks[app->cursor].name, app->maxx/2+1, 1, app->maxy-3, app->maxx);
		
		move(y_name+1, app->maxx/2+1);
		attron(A_REVERSE);
		printw("Description:");
		attroff(A_REVERSE);
		move(y_name+2, app->maxx/2+1);
		fitPhraseToWindow(app->tasks[app->cursor].desc, app->maxx/2+1, y_name+2, app->maxy-3, app->maxx);
		
	}
	
	for (int i=0; i<app->maxy-3; i++)
	{
		move(i, app->maxx/2);
		printw("|");
	}
	move(app->maxy-3, 0);
	for(int i=0; i<app->maxx; i++)
		printw("=");
	
	move(app->maxy-2, app->maxx/2-9);
	printw(" R - remove entry");
	
	move(app->maxy-2, 0);
	printw(" W - move up");
	
	move(app->maxy-1, 0);
	printw(" S - move down");
	
	move(app->maxy-2, app->maxx-17);
	printw(" Q - done/undone");
	
	move(app->maxy-1, app->maxx-17);
	printw(" E - add entry");
	
	
	
	return 0;
}

int handleInput(state * app)
{
	char input = app->input;
	
	if(input == 'w' || input == 'W')
	{
		if(app->cursor > 0)
		{
			app->cursor--;
		}
	}
	
	if(input == 's' || input == 'S')
	{
		if(app->cursor < app->entry_count-1)
		{
			app->cursor++;
		}
	}
	
	if(input == 'e' || input == 'E')
	{
		app->screen = 1;
	}
	
	if(input == 'q' || input == 'Q')
	{
		app->tasks[app->cursor].done = !app->tasks[app->cursor].done;
		saveState(app);
	}
	
	if(input == 'r' || input == 'R')
	{
		if(app->entry_count > 0) deleteEntry(app, app->cursor);
		saveState(app);
	}
	
	return 0;
}

state * appSetup()
{
	state * new_app = new state;
	
	string temp;
	
	fstream file;
	file.open("todo_save.txt");
	if(file.is_open())
	{
		getline( file, temp );
		new_app->entry_count = stoi(temp);
		new_app->tasks = new entry[new_app->entry_count];
		
		if( !file.fail() )
		{
			for(int i=0; i<new_app->entry_count; i++)
			{
				getline( file, new_app->tasks[i].name );
				getline( file, new_app->tasks[i].desc );
				getline( file, temp );
				new_app->tasks[i].done = stoi(temp);
			}
			
		}
		else
		{
			file.close();
			file.open("todo_save.txt", ios::out | ios::trunc);
			file.close();
			new_app -> entry_count = 0;
		}
		
	}
	else
	{
		new_app -> entry_count = 0;
	}
	new_app -> cursor = 0;
	new_app -> screen = 0;
	new_app -> scroll = 0;
	new_app -> input = ' ';
	new_app -> new_entry = nullptr;
	
	return new_app;
}

int screenSetup()
{
	initscr();
	noecho();
	refresh();
	
	return 0;
}
