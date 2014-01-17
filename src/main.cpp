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
/*-----------------------------------------------------------------------*/

/**
 * 
 *	@mainpage Kasterborous
 *	@tableofcontents
 *
 *	@Brief 	Main controls the interfacing between output(UI) and the 
 *    			simulator thread.
 *    
 *
 */
/* ----------------------------------------------------------------------*/

//STD libraries:
#include <thread>
#include <mutex>
#include <string>		
#include <memory>
#include <atomic>

#include "../build/kasterborous.h"
#include "agentdomain.h"
#include "eventqueue.h"
#include "nestene.h"
#include "ID.h"
#include "output.h"
#include "utility.h"

//initialize the is values
int ID::aID = 0;
unsigned long long ID::eID = 0;
unsigned long long ID::tmu = 0;
unsigned long long ID::nID = 0;

//Thread stuff:
std::atomic_bool simDone;
std::shared_ptr<AgentDomain> agentdomain;
std::thread* runThread = NULL;
//variables for getting auton placement data:
std::list<double> sylist; std::list<double> sxlist;
std::list<double> lylist; std::list<double> lxlist;
std::list<double> aylist; std::list<double> axlist;
double width, height;
bool generated = false;


/**
 * Runs the simulation.
 * Start a simulation run, 
 * locked with a mutex as it is intended to be run in a seperate thread.
 * @param runtime number of seconds the simulation should run for
 * @param agentdomain 'smart' pointer to the agentdomain that will be 
 * responsible for the simulation run.
 */
void startSimThread(int runTime, std::shared_ptr<AgentDomain> agentdomain){
	agentdomain->runSimulation(runTime);
	simDone = true;
	generated = false;
}

void clearPlacementData();


int main(int argc, char *argv[])
{
	//instanciate the output singleton:
	Output::Inst();
	//Handle input commands:
	std::string s_filename = "frog.lua";
	std::string s_luaAmount = "0", s_screamerAmount = "0", s_listenerAmount= "0";
	std::string s_macroFactor = "1000", s_timeResolution = "0.000001";
	std::string s_cmd = "run", s_height = "400", s_width = "600", s_time = "10000";	
	*argv++;
	for(int i = 1; i < argc; i++){
		std::string param = *argv;
		if (param.compare("-l")==0){
			if(*argv++ != NULL){
				//Output::Inst()->kprintf("%s", *argv++);
				s_listenerAmount = *argv++;
				i++;
			}	
		} else if(param.compare("-s") == 0){
			if(*argv++ != NULL){
				s_screamerAmount = *argv++;
				i++;
			}
		} else if(param.compare("-L") == 0){
			if(*argv++ != NULL){
				s_luaAmount = *argv++;
				i++;
			}
		}else if(param.compare("-f") == 0){
			if(*argv++ != NULL){
				s_filename = *argv++;
				i++;
			}
		}else if(param.compare("-m") == 0){
			if(*argv++ != NULL){
				s_macroFactor = *argv++;
				i++;
			}
		}else if(param.compare("-r") == 0){
			if(*argv++ != NULL){
				s_timeResolution = *argv++;
				i++;
			}
		}else if(param.compare("-w") == 0){
			if(*argv++ != NULL){
				s_width = *argv++;
				i++;
			}
		}else if(param.compare("-h") == 0){
			if(*argv++ != NULL){
				s_height = *argv++;
				i++;
			}
		}else if(param.compare("-t") == 0){
			if(*argv++ != NULL){
				s_time = *argv++;
				i++;
			}
		}else if(param.compare("-c") == 0){
			if(*argv++ != NULL){
				s_cmd = *argv++;
				i++;
			}
		}
	}

	Output::Inst()->setFields(s_filename, s_luaAmount, s_screamerAmount, s_listenerAmount, s_macroFactor, s_timeResolution, s_cmd, s_height, s_width, s_time);


	//define the command strings:
	simDone = true;
	std::string runSim = "run";
	std::string generateEnv = "gen";
	std::string generateEnvSquare = "run-L";
	std::string generateEnvListenerSquare = "run-l";

	keypad(stdscr,TRUE);

	agentdomain.reset(new AgentDomain);

	int ch;

	while((ch = getch()) != 'Q'){		
		switch(ch){			
			case KEY_F(1):
				//Activate input mode:	
				Output::Inst()->keyHandler(MODE_INPUT);
				break;
			case KEY_F(2):
				//Activate running mode:
				Output::Inst()->keyHandler(MODE_RUNNING);
				break;
			case KEY_F(3):
				//if(generated){
					if(agentdomain->checkEnvPresence()){
						Output::Inst()->generateMapPanels(sylist,sxlist,lylist,lxlist,aylist,axlist,height,width);
						Output::Inst()->keyHandler(MODE_MAP);
					}else
						Output::Inst()->kprintf("No environment to render, (hint:cmd 'gen')\n");
				//}
				break;
			case KEY_F(5) :
				if(!simDone){
					Output::Inst()->kprintf("Simulator still running (F6 to cancel)\n");

				}else if(Output::Inst()->getCMode() != MODE_MAP){
					//initiate run variables:
					int screamerAmount, listenerAmount, luaAmount;
					int macroStepFactor, nestSquareAmount, runtime;
					double microStepRes;
					std::string filename, command;
					//Retrieve input data:
					Output::Inst()->getInputData(screamerAmount, listenerAmount,
							luaAmount, nestSquareAmount, width, height, runtime,
							microStepRes, macroStepFactor,
							filename, command);

					Output::Inst()->kprintf("Executing CMD:\t%s\n", command.c_str());

					if(!generated){
						agentdomain.reset(new AgentDomain);
					}

					if((runSim.compare(command)==0)){
						simDone = false;
						Output::Inst()->kprintf("Starting Simulation Run (F6 to cancel)\n");
						if(!agentdomain->checkEnvPresence()){
							generated = true;
							Output::Inst()->kprintf("No environment found, generating a new one.\n");
							clearPlacementData();
							agentdomain->generateEnvironment(width,height,nestSquareAmount,
									listenerAmount,screamerAmount,luaAmount,
									microStepRes,macroStepFactor,filename);
							agentdomain->retrievePopPos(sylist,sxlist,lylist,lxlist,aylist,axlist,width,height);
							Output::Inst()->kprintf("Environment generated.\n");
						}
						//start the simulation thread:
						runThread = new std::thread(startSimThread,runtime, agentdomain);
						Output::Inst()->keyHandler(MODE_RUNNING);

					}else if(generateEnv.compare(command)==0){
						generated = true;
						Output::Inst()->kprintf("Generating Environment.\n");
						clearPlacementData();
						agentdomain->generateEnvironment(width,height,nestSquareAmount,
								listenerAmount,screamerAmount,luaAmount,
								microStepRes,macroStepFactor,filename);
						agentdomain->retrievePopPos(sylist,sxlist,lylist,lxlist,aylist,axlist,width,height);
						Output::Inst()->kprintf("Environment generated, %d.\n", luaAmount);

					}else if(generateEnvSquare.compare(command)==0){
						generated = true;
						simDone = false;
						Output::Inst()->kprintf("Generating Square Environment.\n");
						clearPlacementData();
						agentdomain->generateSquaredEnvironment(width,height,nestSquareAmount,
								luaAmount,microStepRes,macroStepFactor,filename);
						agentdomain->retrievePopPos(sylist,sxlist,lylist,lxlist,aylist,axlist,width,height);
						Output::Inst()->kprintf("Squared LUA Environment generated, %d.\n", luaAmount);

						runThread = new std::thread(startSimThread,runtime, agentdomain);
						Output::Inst()->keyHandler(MODE_RUNNING);

					}else if(generateEnvListenerSquare.compare(command)==0){
						generated = true;
						simDone = false;
						Output::Inst()->kprintf("Generating Square Environment.\n");
						clearPlacementData();
						agentdomain->generateSquaredListenerEnvironment(width,height,nestSquareAmount,
								listenerAmount,microStepRes,macroStepFactor);
						agentdomain->retrievePopPos(sylist,sxlist,lylist,lxlist,aylist,axlist,width,height);
						Output::Inst()->kprintf("Squared Listener Environment generated, %d.\n", listenerAmount);

						runThread = new std::thread(startSimThread,runtime, agentdomain);
						Output::Inst()->keyHandler(MODE_RUNNING);

					}
				}
				break;
			case KEY_F(6) :
				if(runThread != NULL && runThread->joinable()){
					agentdomain->stopSimulation();
					runThread->join();
					delete runThread;
					runThread = NULL;
				}
				break;
			case KEY_F(7) :
				if(agentdomain != NULL && simDone){
					agentdomain->saveExternalEvents("savefile");
				}
			default:
				Output::Inst()->keyHandler(ch);
				break;
		}

	}	
	if(runThread != NULL && runThread->joinable()){
		agentdomain->stopSimulation();
		runThread->join();
	}
	Output::Inst()->kprintf("Clearing eventqueue data");
	agentdomain.reset();
	endwin();	
	delete runThread;
	return 0;
}

/*
 * Clears the placement lists.
 */
void clearPlacementData(){
	sylist.clear();
	sxlist.clear();
	lylist.clear();
	lxlist.clear();
	aylist.clear();
	axlist.clear();
}

