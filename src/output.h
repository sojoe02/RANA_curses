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
#ifndef OUTPUT_H
#define OUTPUT_H

#include<string>
#include<vector>
#include<list>
#include<ncurses.h>
#include<form.h>
#include<panel.h>
#include<mutex>

#define MODE_START	510
#define MODE_INPUT 	520
#define MODE_OUTPUT	530
#define MODE_MAP	540
#define	MODE_VERBOSE 	550
#define MODE_RUNNING	550
#define INPUT_UP	560
#define INPUT_DOWN	570
#define SIM_RUNNING     590

/**
 * This class controls all output to the screen via the Ncurses framework.
 *
 * 			
 * It is implemented with the singleton pattern so it can be
 * called from everywhere with both consistency and safety.
 * Every public method with access to the framebuffer via Ncurses
 * is protected by the outputMutex in order to prevent screen
 * corruption.
 *
 * @author Soeren Vissing Joergensen
 * @email sojoe02@gmail.com 
 */

class Output
{
	public:

		static Output* Inst();
		//function for writing msg to the current active ouput box, if enabled.
		void kprintf(const char* msg, ...);
		//function for handling information, so information can be treated differently
		//than debug stuff:
		void kInfo(std::string msg);
		//change the current active mode:	
		void changeMode(int mode);
		//Will refresh the screen based on current mode:
		void resetAll();
		void updateScr();
		void keyHandler(int ch);
		void progressBar(unsigned long long current ,unsigned long long maximum);
		void clearProgressBar();

		void updateStatus(unsigned long long ms, unsigned long long eventInit,
	       		unsigned long long internalEvents, unsigned long long externalEvents);
		//get the data from the input fields, returns 0 if successfull:
		void getInputData(int &screamerAmount, int &listenerAmount, int &luaAmount,
				int &nestSquareAmount, double &width, double &height, int &runtime, 
				double &microStepRes, int &macroStepRes,
				std::string &filename, std::string &command);
		
		void generateMapPanels(std::list<double> sylist, std::list<double> sxlist,
		std::list<double> lylist, std::list<double> lxlist,
		std::list<double> aylist, std::list<double> axlist, double yMax, double xMax);
		int getCMode();
		void updatePercentageDone(unsigned long long current, unsigned long long maximum);
		void setFields(std::string s_filename, std::string s_luaAmount, std::string s_screamerAmount, std::string s_listenerAmount, std::string s_macroFactor, std::string s_timeResolution, std::string s_cmd, std::string s_height, std::string s_width, std::string s_time);
		
	private:

		Output();
		Output(Output const&){};
		Output& operator=(Output const&){};
		static Output* output; //the instance of the output singleton
		//Mode of the screen
		//this should be set whenever the simulator changes mode:
		int c_mode;
		//Run mode can cange between these different windows:
		//Windows/pads: 
		//Verbose window, shows all debug information:
		WINDOW *r_output_win;
		//Runnings panels and windows fields etc.:		
		PANEL *rRunningPanel;		
		WINDOW *rRunningMainWin;
		WINDOW *rRunningOutputWin;
		WINDOW *rRunningProgressWin;
		WINDOW *rRunningEchoWin;
		WINDOW *rRunningStatusWin;
		//FORM *statusForm;
		//FIELD *status[5];

		//Mode input panel, windows etc:
		PANEL *rInputPanel;
		WINDOW *rInputMainWin;
		WINDOW *rInputOutputWin;
		WINDOW *rInputEchoWin;
		WINDOW *rFormWin;
		WINDOW *rRunWin;		
		WINDOW *rDebugWin;
		FORM *inputForm;
		FORM *labelForm;
		FIELD *input[12];
		FIELD *label[12];
		
		//Panels for the map and windows, one for each 
		//auton type, and a final one for all of them
		PANEL *rMapPanels[4];
		PANEL *rMapTop;
		WINDOW *rMapBorderWin[4];
		WINDOW *rMapWindows[4];

		//Functions to handle the different modes:
		void modeInput();
		void modeRunning();
		void handleInput(int ch);
		void handleMap(int ch);
		void mapRender(std::list<double> xList, std::list<double> yList, 
				double yMax, double xMax, int mapIndex);

		//Buffering vectors:
		//Debug buffer
		std::vector<std::string> *debugVector;
		//The info buffer
		std::vector<int> *infoVector;
		//Keep track of the windows curser position.
		int currentDebugLine;
		int currentInfoLine;
		/*
		 * Different utility functions:
		 */
		void setFields();	// Setup the fields for the inputFormWin
		void setInputFormWin(int height, int width); // Setup the inputFormWin
		//void setStatusFields();
		//void setStatusFormWin(int height, int width);
		//
		//Mutex set on all public methods, and the constructor:
		//This ensures complete singleton thread safety,
		//which prevents screen corruption, strange crashes etc.
		std::mutex outputMutex;

};
#endif // OUTPUT_H
