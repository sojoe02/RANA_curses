//--begin_license--
//
//Copyright 	2013 	Søren Vissing Jørgensen.
//			2014	Søren Vissing Jørgensen, Center for Biorobotics, Sydansk Universitet MMMI.  
//
//This file is part of RANA.
//
//RANA is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//RANA is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with RANA.  If not, see <http://www.gnu.org/licenses/>.
//
//--end_license--


//Ncurses:
#include <ncurses.h>
#include <form.h>
#include <panel.h>
//THREAD:
#include <mutex>
//STL:
#include <list>
//STD:
#include <cstring>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <climits>
#include <cfloat>

#include "output.h"
#include "../build/kasterborous.h"

Output* Output::output;

/**
 * Singleton accessor.
 * Static method for accessing the singletons instance
 * this call is NOT threadsafe, due to performance concerns
 * So it must be called at least once from main thread 
 * before any other thread calls is.
 */
Output* Output::Inst()
{
	if(!output)
		output = new Output();

	return output;
}

/**
 * Outputs private constructor.
 * sets up the different ncurses modes, colors etc.
 */
	Output::Output()
:currentDebugLine(0), currentInfoLine(0)
{
	std::lock_guard<std::mutex> lock(outputMutex);
	//init all the Ncurses stuff:
	initscr();
	keypad(stdscr,TRUE);
	cbreak();
	noecho();
	if(has_colors()){
		start_color();
		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
		init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4, COLOR_BLUE,    COLOR_BLACK);
		init_pair(5, COLOR_CYAN,    COLOR_BLACK);
		init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(7, COLOR_WHITE,   COLOR_BLACK);
	}
	modeRunning();
	modeInput();
	update_panels();
	doupdate();
	c_mode = MODE_INPUT;
	debugVector = new std::vector<std::string>;
	infoVector = new std::vector<int>;
}

/**
 * This function handles all input from the user.
 * After the main() method done it's user input 
 * interpretation
 * @param ch input character.
 */
void Output::keyHandler(int ch){
	std::lock_guard<std::mutex> lock(outputMutex);
	refresh();
	switch(ch){
		case KEY_RESIZE:
			resetAll();
			break;
		case MODE_RUNNING:
			curs_set(0);
			c_mode = MODE_RUNNING;
			top_panel(rRunningPanel);
			update_panels();
			doupdate();
			updateScr();
			break;
		case MODE_INPUT:
			curs_set(1);
			c_mode = MODE_INPUT;
			top_panel(rInputPanel);
			update_panels();
			doupdate();
			updateScr();
			break;
		case MODE_MAP:
			curs_set(1);
			c_mode = MODE_MAP;
			top_panel(rMapTop);
			update_panels;
			doupdate();
			updateScr();
			break;
		default : 
			if(c_mode == MODE_INPUT)
				handleInput(ch);
			else if(c_mode == MODE_MAP)
				handleMap(ch);
			break;
	}
	//move the curser to the last active inputForm field, 
	//and position:
	if(c_mode == MODE_INPUT){
		form_driver(inputForm, REQ_END_LINE);
	}
	//refresh();
}

/**
 * Handles user input when in 'input mode'.
 * @param ch which is the input character
 */
void Output::handleInput(int ch){
	switch(ch){
		case KEY_UP :
			form_driver(inputForm,REQ_PREV_FIELD);
			form_driver(inputForm,REQ_END_LINE);
			break;
		case KEY_DOWN :
			form_driver(inputForm,REQ_NEXT_FIELD);
			form_driver(inputForm,REQ_END_LINE);
			break;
		case KEY_BACKSPACE :
			form_driver(inputForm, REQ_DEL_LINE);
			break;
		default	:
			form_driver(inputForm,ch);
			wrefresh(rInputMainWin);
			break;
	}
	wrefresh(rFormWin);
}

/**
 * Handle user input when in 'map mode'.
 * @param ch input character
 */
void Output::handleMap(int ch){
	switch(ch){
		case 9 :
			rMapTop = (PANEL *)panel_userptr(rMapTop);
			top_panel(rMapTop);
			update_panels();
			doupdate();
			//refresh();
			break;
	}
}

/**
 * Setup the 'running mode'.
 * Initialize the window sizes etc.
 */
void Output::modeRunning(){
	refresh();
	/*---------------------------------------------------------------------------
	 * Initialize the windows and their subwindows
	 *----------------------------------------------------------------------------*/
	int rIW_h = LINES;
	int rIW_w = 3*COLS/6;
	int rIW1_h = LINES;
	int rIW1_w = COLS/6;
	int rIW2_h = LINES;
	int rIW2_w = 2*COLS/6;       

	rRunningMainWin = newwin(LINES,COLS,0,0); 

	rRunningOutputWin	= subwin(rRunningMainWin,rIW_h-2,rIW_w-2,1,1);
	rRunningProgressWin 	= subwin(rRunningMainWin,rIW1_h-2,rIW1_w-2,1,3*COLS/6);
	rRunningStatusWin 	= subwin(rRunningMainWin,rIW2_h-2,rIW2_w-2,1,4*COLS/6);

	rRunningEchoWin = derwin(rRunningOutputWin,rIW1_h-4, rIW_w-4,1,1);	

	//make the sub windows:
	scrollok(rRunningEchoWin,TRUE);
	idlok(rRunningEchoWin,TRUE);

	wattron(rRunningMainWin,COLOR_PAIR(1));
	box(rRunningMainWin,0,0);
	mvwprintw(rRunningMainWin,0,1,"Running Panel");
	wattroff(rRunningMainWin,COLOR_PAIR(1));

	wattron(rRunningOutputWin,COLOR_PAIR(2));
	box(rRunningOutputWin,0,0);
	wattroff(rRunningOutputWin,COLOR_PAIR(2));
	wattron(rRunningStatusWin,COLOR_PAIR(2));	
	box(rRunningStatusWin,0,0);
	wattroff(rRunningStatusWin,COLOR_PAIR(2));
	//setStatusFormWin(rIW2_h-1,rIW2_w-1);	
	wattron(rRunningStatusWin,COLOR_PAIR(2));
	wattron(rRunningStatusWin,A_BOLD);
	mvwprintw(rRunningStatusWin, 1, 1,  "Current TMU:");
	mvwprintw(rRunningStatusWin, 3, 1,  "Amount of events Initiated:");
	mvwprintw(rRunningStatusWin, 5, 1,  "Internal events in the queue");
	mvwprintw(rRunningStatusWin, 7, 1,  "External events in the queue");
	mvwprintw(rRunningStatusWin, 9, 1,  "Percentage done");
	wattroff(rRunningStatusWin,COLOR_PAIR(2));
	wattroff(rRunningStatusWin,A_BOLD);
	//box(rRunningProgressWin,0,0);
	rRunningPanel = new_panel(rRunningMainWin);	
	//Do a refresh:
	//wrefresh(rRunningMainWin);
	//refresh();
}

/**
 * Setup input mode.
 * @see Output::modeRunning()
 */
void Output::modeInput(){
	//clear the screen and make the windows:
	//clearscr();
	refresh();
	/* ---------------------------------------------------------------------------
	 * Initialize the windows and their subwindows
	 *----------------------------------------------------------------------------*/
	int rIW_h = 2*LINES/3;
	int rIW_w = COLS;
	int rIW1_h = LINES/3;
	int rIW1_w = COLS; 

	rInputMainWin = newwin(LINES,COLS,0,0);
	rInputOutputWin	 = subwin(rInputMainWin,rIW1_h,rIW1_w-2,2*LINES/3,1);
	rInputEchoWin	 = derwin(rInputOutputWin,rIW1_h-2,rIW1_w-4, 1,1);
	//make the sub wwindows:
	scrollok(rInputEchoWin,TRUE);
	idlok(rInputEchoWin,TRUE);

	//keypad(rInputWin, TRUE);
	wattron(rInputMainWin,COLOR_PAIR(2));
	mvwprintw(rInputMainWin,2,COLS/3 , "Kasterborous Version %d.%d\n",
			Kasterborous_VERSION_MAJOR,
			Kasterborous_VERSION_MINOR);
	wattroff(rInputMainWin,COLOR_PAIR(2));

	wattron(rInputMainWin,COLOR_PAIR(4));
	box(rInputMainWin,0,0);
	mvwprintw(rInputMainWin,0,1,"Input Panel");
	wattroff(rInputMainWin,COLOR_PAIR(4));


	wattron(rInputOutputWin,COLOR_PAIR(3));
	box(rInputOutputWin,0,0);
	wattroff(rInputOutputWin,COLOR_PAIR(3));
	//box(rFormWin,0,0);
	/*---------------------------------------------------------------------------- 
	 *Initialize the forms:
	 *----------------------------------------------------------------------------*/
	setInputFormWin(rIW_h-5,rIW_w-2);
	box(rFormWin,0,0);

	rInputPanel = new_panel(rInputMainWin);
}


/**
 * Will update the currently active screen.
 * Used by all functions writing to the framebuffer
 */
void Output::updateScr(){
	switch(c_mode){
		case MODE_INPUT:
			wrefresh(rInputMainWin);
			break;
		case MODE_RUNNING:
			wrefresh(rRunningStatusWin);
			wrefresh(rRunningProgressWin);
			wrefresh(rRunningMainWin);
			break;
		default:
			break;
	}
}

/*
 * Refresh the screen based on the current mode, for resizing etc:
 */
void Output::resetAll(){
	//clean everything up!	
	//the input:
	delwin(rFormWin);
	delwin(rInputEchoWin);
	delwin(rInputOutputWin);
	delwin(rInputMainWin);
	del_panel(rInputPanel);
	//the output:
	delwin(rRunningEchoWin);
	delwin(rRunningOutputWin);
	delwin(rRunningStatusWin);
	delwin(rRunningMainWin);
	del_panel(rRunningPanel);
	refresh();
	//the maps:
	//delwin()
	//reinitiate the modes:
	modeRunning();
	modeInput();	
	update_panels();
	doupdate();
	c_mode = MODE_INPUT;
}


/*------------------------------------------------------------------
 * Utility functions,
 * -get user input data 
 * -printf wrapper
 * -progress bar render
 *------------------------------------------------------------------*/

/**
 * Retrieval of user inputs.
 * Writes all the inputFields data to the given variable adresses.
 * @param screamerAmount number of screamers.
 * @param listenerAmount number of listeners.
 * @param luaAmount number of lau autons.
 * @param nestSquareAmount number of nestenes squared.
 * @param width of the area(m).
 * @param height of the area(m).
 * @param microStepRes micro step resolution in seconds.
 * @param macroStepFactor macro step factor.
 * @return void
 * */
void Output::getInputData(int &screamerAmount, int &listenerAmount, int &luaAmount,
		int &nestSquareAmount, double &width, double &height, int &runtime,
		double &microStepRes, int &macroStepFactor,std::string &filename, std::string &command){
	//The autons:
	screamerAmount = atoi(field_buffer(input[0],0));
	listenerAmount = atoi(field_buffer(input[1],0));
	luaAmount = atoi(field_buffer(input[2],0));
	//The nestenes:
	nestSquareAmount = atoi(field_buffer(input[3],0));
	//Size of the sim:
	width = atof(field_buffer(input[4],0));
	height = atof(field_buffer(input[5],0));
	//Time the simulator runs
	runtime = atoi(field_buffer(input[6],0));
	//Step resolutions:
	microStepRes = atof(field_buffer(input[7],0));
	macroStepFactor = atoi(field_buffer(input[8],0));
	//The filename and command, clear all white spaces!
	std::string c  = field_buffer(input[10],0);
	std::string tmp;
	for(int i = 0; i < c.length(); i++){
		if(c[i] != 32)
			tmp.append(1,c[i]);	
	}
	command = tmp;
	c = field_buffer(input[9],0);
	tmp = "";
	for(int i = 0; i < c.length(); i++){
		if(c[i] != 32)
			tmp.append(1,c[i]);
	}
	filename = tmp;
}

/**
 * printf wrapper function.
 * printf wrapper which writes the msg on the currently active output screen.
 * @param msg
 * @param variable list
 * @see vwprintw
 * @see printf
 */
void Output::kprintf(const char* msg, ...){
	std::lock_guard<std::mutex> lock(outputMutex);
	va_list args;
	va_start(args,msg);
	switch(c_mode){
		case MODE_INPUT:
			vwprintw(rInputEchoWin,msg,args);
			wrefresh(rInputEchoWin);
			break;
		case MODE_RUNNING:
			vwprintw(rRunningEchoWin,msg,args);
			wrefresh(rRunningEchoWin);
			break;
		default:
			break;
	}
	va_end(args);
}

/**
 * ASCII progress bar.
 * Renders a progress bar in the 'Running' panel, fills ~1/6 of the screen width
 * colours are: red progress < 30%, yellow (30%< progress <70%),
 * green progress > 70%.
 * @param current current value of progress.
 * @param maximum value of progress, ie when it's done.
 */
void Output::progressBar(unsigned long long current, unsigned long long maximum){
	std::lock_guard<std::mutex> lock(outputMutex);

	//if(c_mode == MODE_RUNNING){
		int barmaxX, barmaxY, barHeight, barSize;

		//getting the max y,x sizes for the bar:
		getmaxyx(rRunningProgressWin, barmaxY, barmaxX);

		int total = barmaxY * barmaxX;
		int complete = (total * current)/maximum;

		wattron(rRunningProgressWin,A_BOLD);
		curs_set(0);
		int j = 0;
		int k = 0;
		
		for(int i=0; i<complete; i++){
			if(i % (barmaxX) == 0){
				j++; //the row:
				//wmove(rRunningProgressWin, barmaxY-j,0);
				//wclrtoeol(rRunningProgressWin);
				k=0; //the colum:
			}
			if((i*100)/total < 30){
				wattron(rRunningProgressWin,COLOR_PAIR(1));
				mvwprintw(rRunningProgressWin,barmaxY-j,k,"=");
				wattroff(rRunningProgressWin,COLOR_PAIR(1));
			} else if((i*100)/total > 70 ){
				wattron(rRunningProgressWin,COLOR_PAIR(2));
				mvwprintw(rRunningProgressWin,barmaxY-j,k,"=");
				wattroff(rRunningProgressWin,COLOR_PAIR(2));
			} else{
				wattron(rRunningProgressWin,COLOR_PAIR(3));		
				mvwprintw(rRunningProgressWin,barmaxY-j,k,"=");
				wattroff(rRunningProgressWin,COLOR_PAIR(3));
			}
			k++;
		}
		wattroff(rRunningProgressWin,A_BOLD);
		double percentage = (double)(current *100) /(double)maximum;
		mvwprintw(rRunningStatusWin, 10, 1, "%*f",12, percentage);
		updateScr();
	//}
}

void Output::updatePercentageDone(unsigned long long current, unsigned long long maximum){
	std::lock_guard<std::mutex> lock(outputMutex);
	double percentage = (double)(current *100) /(double)maximum;
	mvwprintw(rRunningStatusWin, 10, 1, "%*f",12, percentage);
	updateScr();
}

void Output::clearProgressBar(){
	std::lock_guard<std::mutex> lock(outputMutex);
	wclear(rRunningProgressWin);
	updateScr();
	//wrefresh(rRunningStatusWin);
}

/**
 * Status screen control
 * Updates status of different values at fixed points in the status screen
 * in 'Running' mode.
 * @param ms current microstep
 * @param eventInit external events initated
 * @param internalEvents number or internal events in the eventqueue
 * @param externalEvents number of external events in the eventqueue
 */
void Output::updateStatus(unsigned long long ms, unsigned long long eventInit,
		unsigned long long internalEvents, unsigned long long externalEvents){
	std::lock_guard<std::mutex> lock(outputMutex);
	//curs_set(0);
	switch(c_mode){
		case MODE_RUNNING :{
					   //clear the lines:
					   for(int i = 2; i <= 8 ;){
						   wmove(rRunningStatusWin, i,1);	
						   wclrtoeol(rRunningStatusWin);
						   i += 2;
					   }
					   wattron(rRunningStatusWin,COLOR_PAIR(2));	
					   box(rRunningStatusWin,0,0);
					   wattroff(rRunningStatusWin,COLOR_PAIR(2));
					   int padding = 12;
					   //then the outputs:
					   mvwprintw(rRunningStatusWin, 2, 1, "%*llu", padding,ms);
					   mvwprintw(rRunningStatusWin, 4, 1, "%*i", padding,eventInit);
					   mvwprintw(rRunningStatusWin, 6, 1, "%*i", padding,internalEvents);
					   mvwprintw(rRunningStatusWin, 8, 1, "%*i", padding,externalEvents);
					   //wrefresh(rRunningStatusWin);
				   }
				   break;
		default:
				   break;
	}
	//curs_set(1);
	doupdate();
}

/** 
 * Generate Map panels
 * Generates the maps based on position data received on the three populations
 * one map for each population and one for all of the different populations.
 * @param sylist y positions of all Screamer autons
 * @param sxlist x positions of all Screamer autons
 * @param lylist y positions of all Listener autons
 * @param lxlist x positions of all Listener autons
 * @param aylist y positions of all Lua autons
 * @param axlist x positions of all Lua autons
 * @param yMax maps maximum height
 * @param xMax maps maximum width
 */
void Output::generateMapPanels(std::list<double> sylist, std::list<double> sxlist,
		std::list<double> lylist, std::list<double> lxlist,
		std::list<double> aylist, std::list<double> axlist, double yMax, double xMax){
	std::lock_guard<std::mutex> lock(outputMutex);

	//Generate the map panels:
	for(int i=0; i<4;i++){
		rMapBorderWin[i] = newwin(LINES-2,COLS-2,1,1);
		wattron(rMapBorderWin[i],COLOR_PAIR(i));		
		box(rMapBorderWin[i],0,0);
		wattroff(rMapBorderWin[i],COLOR_PAIR(i));
		rMapWindows[i] = derwin(rMapBorderWin[i],LINES-4,COLS-4,1,1);
		rMapPanels[i] = new_panel(rMapBorderWin[i]);
	}

	mvwprintw(rMapBorderWin[0],0,1,"Screamer Placement, area: %ix%i[m]", (int)xMax, (int)yMax);
	mvwprintw(rMapBorderWin[1],0,1,"Listener Placement, area: %ix%i[m]", (int)xMax, (int)yMax);
	mvwprintw(rMapBorderWin[2],0,1,"LUA Placement, area: %ix%i[m]", (int)xMax, (int)yMax);
	mvwprintw(rMapBorderWin[3],0,1,"ALL Placement, area: %ix%i[m]", (int)xMax, (int)yMax);

	set_panel_userptr(rMapPanels[0],rMapPanels[1]);	
	set_panel_userptr(rMapPanels[1],rMapPanels[2]);	
	set_panel_userptr(rMapPanels[2],rMapPanels[3]);	
	set_panel_userptr(rMapPanels[3],rMapPanels[0]);

	update_panels();	 
	//Populate the maps with data:
	mapRender(sylist,sxlist,yMax,xMax,0);
	mapRender(lylist,lxlist,yMax,xMax,1);
	mapRender(aylist,axlist,yMax,xMax,2);


	std::list<double> allY;
	allY.insert(allY.end(), sylist.begin(), sylist.end());
	allY.insert(allY.end(), lylist.begin(), lylist.end());
	allY.insert(allY.end(), aylist.begin(), aylist.end());
	std::list<double> allX;
	allX.insert(allX.end(), sxlist.begin(), sxlist.end());
	allX.insert(allX.end(), lxlist.begin(), lxlist.end());
	allX.insert(allX.end(), axlist.begin(), axlist.end());

	mapRender(allY,allX,yMax,xMax,3);
	rMapTop = rMapPanels[3];
	top_panel(rMapTop);
	update_panels();
	doupdate();

}

/**
 * Renders the a population distribution map.
 * Will render the populations given by positions in the x,y lists given.
 * @param ylist y positions
 * @param xlist x positions
 * @yMax maps height.
 * @xMax maps width.
 * @mapIndex the index of this map in the mappanel, which can contain several maps.
 */
void Output::mapRender(std::list<double> ylist, std::list<double> xlist, double yMax, double xMax , int mapIndex){
	std::list<double>::iterator xit;
	std::list<double>::iterator yit;
	double cX, cY;
	int wX, wY;
	getmaxyx(rMapWindows[mapIndex], wY, wX);
	//Make a map matrix
	int mapValues[wY][wX];
	//fill it with 0s
	for(int i = 0; i<wY; i++){
		for(int j = 0; j<wX; j++){
			mapValues[i][j] = 0;
			//kprintf("%i",mapValues[i][j]);
		}
	}

	for(xit=xlist.begin(), yit=ylist.begin(); xit!=xlist.end();++xit,++yit){
		double tmpX = *xit;
		double tmpY = *yit;
		cX = (tmpX*wX)/xMax;
		//if( (cX - (int)cX) > 0.5)
		//	cX++;
		cY = (tmpY *wY)/yMax;
		//if( (cY - (int)cY) > 0.5)
		//printf("positions read are, %f,%f\n", cX, cY);

		//	cY++;
		//kprintf("testing maprender y:%i, x:%i", (int)cX, (int)cY);

		mapValues[(int)cY-0][(int)cX-0]++;
	}

	for(int i=0; i<wY;i++){
		for(int j=0; j<wX; j++){
			//kprintf("%i",mapValues[i][j]);
			if(mapValues[i][j] == 0){
				wattron(rMapWindows[mapIndex],COLOR_PAIR(4));
				mvwprintw(rMapWindows[mapIndex],i,j,".");
				wattroff(rMapWindows[mapIndex],COLOR_PAIR(4));

			}else {
				wattron(rMapWindows[mapIndex],COLOR_PAIR(2));
				wattron(rMapWindows[mapIndex],A_BOLD);	
				mvwprintw(rMapWindows[mapIndex], i, j, "%i", mapValues[i][j]);
				wattroff(rMapWindows[mapIndex],COLOR_PAIR(2));
				wattroff(rMapWindows[mapIndex],A_BOLD);
			}

		}
	}
	wrefresh(rMapWindows[mapIndex]);

}

/**
 * Return the current mode
 * @return c_mode, the current mode the simulator UI is in
 */
int Output::getCMode(){
	return c_mode;
}

/**
 * Setup of the input form windows.
 * Involves alot of field setup code.
 * @param heigh char height of the form window
 * @param width char width of the form window
 */
void Output::setInputFormWin(int height, int width){

	rFormWin = derwin(rInputMainWin,height,width,4,1);

	setFields();	

	inputForm = new_form(input);
	labelForm = new_form(label);
	//Calculate the size of the form, and make a subwindow for it:
	int fRows, fCols;
	scale_form(inputForm,&fRows,&fCols);
	set_form_win(inputForm, rFormWin);
	set_form_sub(inputForm, derwin(rFormWin, fRows, fCols,1,25));

	scale_form(labelForm,&fRows,&fCols);
	set_form_win(labelForm, rFormWin);
	set_form_sub(labelForm, derwin(rFormWin, fRows, fCols,1,1));
	post_form(inputForm);
	post_form(labelForm);

	wattron(rFormWin,COLOR_PAIR(3));
	mvwprintw(rFormWin,1, 62, "Shortcut Keys:");
	mvwprintw(rFormWin,2, 62, "F1: \tActivate Input Panel");
	mvwprintw(rFormWin,3, 62, "F2: \tActivate Run Panel");
	mvwprintw(rFormWin,4, 62, "F3: \tASCII map (TAB to cycle)");
	mvwprintw(rFormWin,5, 62, "F5: \tExecute Input Command");
	mvwprintw(rFormWin,6, 62, "F6: \tStop Running Simulation");
	mvwprintw(rFormWin,7, 62, "Ctrl Q: \tquit simulation");
	wattroff(rFormWin,COLOR_PAIR(3));

	wrefresh(rFormWin);
}

void Output::setFields(){
	//initialize the fields:
	input[0] = new_field(1,16,0,0,0,0);	//Screamer amount
	input[1] = new_field(1,16,1,0,0,0);	//Listener amount
	input[2] = new_field(1,16,2,0,0,0); 	//Lua amount
	input[3] = new_field(1,16,3,0,0,0);	//Nestene resoultion (n^2)
	input[4] = new_field(1,16,4,0,0,0);	//width(m),x
	input[5] = new_field(1,16,5,0,0,0);	//height(m),y
	input[6] = new_field(1,16,6,0,0,0);     //Realtime sim(s)
	input[7] = new_field(1,16,7,0,0,0);	//microstep resolution(s)
	input[8] = new_field(1,16,8,0,0,0);	//macrostep factor
	input[9] = new_field(1,32,9,0,0,0);	//Lua filename
	input[10] = new_field(1,32,10,0,0,0);	//CMD:
	input[11] = NULL;

	label[0] = new_field(1,30,0,0,0,0);
	label[1] = new_field(1,30,1,0,0,0);
	label[2] = new_field(1,30,2,0,0,0); 	//Lua amount
	label[3] = new_field(1,30,3,0,0,0);	//Nestene resoultion (n^2)
	label[4] = new_field(1,30,4,0,0,0);	//width(m), x
	label[5] = new_field(1,30,5,0,0,0);	//height(m),y
	label[6] = new_field(1,30,6,0,0,0);     //
	label[7] = new_field(1,30,7,0,0,0);	//microstep resolution(s)
	label[8] = new_field(1,30,8,0,0,0);	//macrostep factor
	label[9] = new_field(1,30,9,0,0,0);	//filename
	label[10] = new_field(1,30,10,0,0,0);
	label[11] = NULL;
	//set field options:
	/*  
	//The input fields:
	set_field_back(input[0],A_UNDERLINE);
	set_field_back(input[1],A_UNDERLINE);
	set_field_back(input[2],A_UNDERLINE);
	set_field_back(input[3],A_UNDERLINE);
	set_field_back(input[4],A_UNDERLINE);
	set_field_back(input[5],A_UNDERLINE);
	set_field_back(input[6],A_UNDERLINE);
	set_field_back(input[7],A_UNDERLINE);
	set_field_back(input[8],A_UNDERLINE);
	set_field_back(input[9],A_UNDERLINE);
	set_field_back(input[10],A_UNDERLINE);

	//the labels:
	set_field_back(input[0], COLOR_PAIR(3));
	set_field_back(input[1], COLOR_PAIR(3));
	set_field_back(input[2], COLOR_PAIR(3));
	set_field_back(input[3], COLOR_PAIR(3));
	set_field_back(input[4], COLOR_PAIR(3));
	set_field_back(input[5], COLOR_PAIR(3));
	set_field_back(input[6], COLOR_PAIR(3));
	set_field_back(input[7], COLOR_PAIR(3));
	set_field_back(input[8], COLOR_PAIR(3));
	set_field_back(input[9], COLOR_PAIR(3));
	set_field_back(input[10], COLOR_PAIR(3));
*/
	//Validation:
	set_field_type(input[0], TYPE_INTEGER,0,0,INT_MAX);
	set_field_type(input[1], TYPE_INTEGER,0,0,INT_MAX);
	set_field_type(input[2], TYPE_INTEGER,0,0,INT_MAX);
	set_field_type(input[3], TYPE_INTEGER,0,0,INT_MAX);
	set_field_type(input[4], TYPE_NUMERIC,6,4.0,FLT_MAX);
	set_field_type(input[5], TYPE_NUMERIC,6,4.0,FLT_MAX);
	set_field_type(input[6], TYPE_INTEGER,0,0.0,INT_MAX);
	set_field_type(input[7], TYPE_NUMERIC,6,0,FLT_MAX);
	set_field_type(input[8], TYPE_INTEGER,0,0,INT_MAX);

	//the labels:
	set_field_back(label[0], COLOR_PAIR(3));
	set_field_back(label[1], COLOR_PAIR(3));
	set_field_back(label[2], COLOR_PAIR(3));
	set_field_back(label[3], COLOR_PAIR(3));
	set_field_back(label[4], COLOR_PAIR(3));
	set_field_back(label[5], COLOR_PAIR(3));
	set_field_back(label[6], COLOR_PAIR(3));
	set_field_back(label[7], COLOR_PAIR(3));
	set_field_back(label[8], COLOR_PAIR(3));
	set_field_back(label[9], COLOR_PAIR(3));
	set_field_back(label[10], COLOR_PAIR(3));

	set_field_buffer(label[0],0,"Screamer Amount");
	set_field_buffer(label[1],0,"Listener amount");
	set_field_buffer(label[2],0,"Lua Amount");
	set_field_buffer(label[3],0,"Nestene resolution");
	set_field_buffer(label[4],0,"Area Width(m)");
	set_field_buffer(label[5],0,"Area Height(m)");
	set_field_buffer(label[6],0,"Time[s]");
	set_field_buffer(label[7],0,"Microstep Res[s]");
	set_field_buffer(label[8],0,"Macro Factor");
	set_field_buffer(label[9],0,"Lua filename");
	set_field_buffer(label[10],0,"CMD(run,gen)");
}


void Output::setFields(std::string s_filename, std::string s_luaAmount, std::string s_screamerAmount, std::string s_listenerAmount, std::string s_macroFactor, std::string s_timeResolution, std::string s_cmd, std::string s_height, std::string s_width, std::string s_time){
	
	set_field_buffer(input[0],0,s_screamerAmount.c_str());
	set_field_buffer(input[1],0,s_listenerAmount.c_str());
	set_field_buffer(input[2],0,s_luaAmount.c_str());
	set_field_buffer(input[3],0,"1");
	set_field_buffer(input[4],0,s_width.c_str());
	set_field_buffer(input[5],0,s_height.c_str());
	set_field_buffer(input[6],0,s_time.c_str());
	set_field_buffer(input[7],0,s_timeResolution.c_str());
	set_field_buffer(input[8],0,s_macroFactor.c_str());
	set_field_buffer(input[9],0,s_filename.c_str());
	set_field_buffer(input[10],0,s_cmd.c_str());
	wrefresh(rFormWin);
}

/*
 *
 void Output::setStatusFormWin(int height, int width){

 setStatusFields();

 statusForm = new_form(status);

//Calculate the size of the form, and make a subwindow for it:
int fRows, fCols;
scale_form(statusForm,&fRows,&fCols);
set_form_win(statusForm, rRunningStatusWin);
set_form_sub(statusForm, derwin(rRunningStatusWin, fRows, fCols,1,1));

post_form(statusForm); 

wrefresh(rRunningStatusWin);
}


void Output::setStatusFields(){
//initialize the status field labels:
status[0] = new_field(1,30,0,0,0,0);	//Events initiated.
status[1] = new_field(1,30,2,0,0,0);	//External Events in Queue
status[2] = new_field(1,30,4,0,0,0); 	//Internal Events in Queue
status[3] = new_field(1,30,6,0,0,0);	//Current Time (s).	

//Set Final Field to NULL(not really neded);
status[4] = NULL;

//set field options:	
set_field_back(status[0], COLOR_PAIR(3));
set_field_back(status[1], COLOR_PAIR(3));
set_field_back(status[2], COLOR_PAIR(3));
set_field_back(status[3], COLOR_PAIR(3));

set_field_buffer(status[0],0,"Events Initiated:");
set_field_buffer(status[1],0,"External Events in Queue:");
set_field_buffer(status[2],0,"Internal Events in Queue:");
set_field_buffer(status[3],0,"Current Time(s):");

}


*/
