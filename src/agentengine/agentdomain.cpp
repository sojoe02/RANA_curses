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
#include <chrono>
#include<climits>
#include "agentdomain.h"
#include "master.h"
#include "phys.h"
#include "output.h"
#include "ID.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

AgentDomain::AgentDomain()
	:mapGenerated(false), stop(false)
	 {
		 Phys::seedMersenne();
}

AgentDomain::~AgentDomain(){
	ID::resetSystem();
	Phys::setCTime(0);
}
/**
 * Checks if an environment has been generated.
 * @return bool false if there is not environment, true if there is.
 * */
bool AgentDomain::checkEnvPresence(){
	return mapGenerated;	
}

void AgentDomain::interpret(std::string function){
}

/**
 * Generates the environment.
 * Upon environment generation the nestenes will be placed, autons will be 
 * assigned to a nestene and placed within it's parameters.
 */
void AgentDomain::generateEnvironment(double width, double height, int resolution,
		int listenerSize, int screamerSize, int LUASize,
		double timeResolution, int macroFactor, std::string filename){

	this->timeResolution = timeResolution;
	this->macroFactor = macroFactor;
	macroResolution = macroFactor * timeResolution;


	Phys::setTimeRes(timeResolution);
	Phys::setCTime(0);
	Phys::setMacroFactor(macroFactor);
	Phys::setEnvironment(width, height);

	master.generateMap(width,height,resolution,timeResolution, macroResolution);
	mapWidth = width;
	mapHeight = height;

	master.populateSystem(listenerSize, screamerSize, LUASize, filename);
	mapGenerated = true;
}


/**
 * Generates the environment.
 * Upon environment generation the nestenes will be placed, autons will be 
 * assigned to a nestene and placed within it's parameters.
 */
void AgentDomain::generateSquaredEnvironment(double width, double height, int resolution,int LUASize,double timeResolution, int macroFactor, std::string filename){

	this->timeResolution = timeResolution;
	this->macroFactor = macroFactor;
	macroResolution = macroFactor * timeResolution;


	Phys::setTimeRes(timeResolution);
	Phys::setCTime(0);
	Phys::setMacroFactor(macroFactor);
	Phys::setEnvironment(width, height);


	master.generateMap(width,height,resolution,timeResolution, macroResolution);

	//std::string filename = "frog.lua";

	mapWidth = width;
	mapHeight = height;

	master.populateSquareSystem(LUASize, filename);
	mapGenerated = true;
}
/**
 * Generates the environment.
 * Upon environment generation the nestenes will be placed, autons will be 
 * assigned to a nestene and placed within it's parameters.
 */
void AgentDomain::generateSquaredListenerEnvironment(double width, double height, int resolution,int listenerSize,double timeResolution, int macroFactor){

	this->timeResolution = timeResolution;
	this->macroFactor = macroFactor;
	macroResolution = macroFactor * timeResolution;


	Phys::setTimeRes(timeResolution);
	Phys::setCTime(0);
	Phys::setMacroFactor(macroFactor);
	Phys::setEnvironment(width, height);


	master.generateMap(width,height,resolution,timeResolution, macroResolution);

	//std::string filename = "frog.lua";

	mapWidth = width;
	mapHeight = height;

	master.populateSquareListenerSystem(listenerSize);
	mapGenerated = true;
}


/**
 * Retrieval of auton positions.
 * Will write the positions of all autons to the std::lists given as arguments.
 * @param sylist y positions of all Screamer autons
 * @param sxlist x positions of all Screamer autons
 * @param lylist y positions of all Listener autons
 * @param lxlist x positions of all Listener autons
 * @param aylist y positions of all Lua autons
 * @param axlist x positions of all Lua autons
 */
void AgentDomain::retrievePopPos(std::list<double> &sylist, std::list<double> &sxlist,
		std::list<double> &lylist, std::list<double> &lxlist,
		std::list<double> &aylist, std::list<double> &axlist,
		double &width, double &height){

	master.retrievePopPos(sylist, sxlist, lylist, lxlist, aylist, axlist);
	width = mapWidth;
	height = mapHeight;
}

/**
 * Runs the simulation.
 * Start a simulation run, this will run a simulation, width the defined macro and micro
 * precision, the sun can be cancelled via the atomic boolean stop. The run will 
 * update the progress bar and status window in the running panel.
 * @param time the amount of seconds the simulation will simulate.
 */
void AgentDomain::runSimulation(int time){
	stop = false;
	//Output::Inst()->kprintf("Running Simulation of: %i[s], with resolution of %f \n", time, timeResolution);

	unsigned long long iterations = (double)time/timeResolution;
	Output::Inst()->clearProgressBar();
	auto start = steady_clock::now();
	auto start2 = steady_clock::now();

	unsigned long long run_time = 0;

	unsigned long long cMacroStep = 0;
	unsigned long long cMicroStep = ULLONG_MAX;
	unsigned long long i = 0, j = 0;

	for(i = 0; i < iterations;){

		Phys::setCTime(i);

		if(i == cMicroStep && cMicroStep != ULLONG_MAX){
			master.microStep(i);		
			//Output::Inst()->kprintf("i is now %lld\n", i);
		}		
		if(i == cMacroStep){
			master.macroStep(i);
			cMacroStep +=macroFactor;
		}		
		i = cMacroStep;
		cMicroStep = master.getNextMicroTmu();

		if( i > cMicroStep){
			i = cMicroStep;
		}	
			
		//Update the status and progress bar screens:		
		auto end = steady_clock::now();

		if(duration_cast<milliseconds>(end-start).count() > 350){
			master.printStatus();
			Output::Inst()->progressBar(cMacroStep,iterations);
			//Output::Inst()->kprintf("i is not : %d\n", i );
			start = end;
		}
		if(stop == true){
			Output::Inst()->kprintf("Stopping simulator at microstep %llu \n", i);
			break;
		}
	}
	master.simDone();
	master.printStatus();
	Output::Inst()->progressBar(i,iterations);
	auto endsim = steady_clock::now();
	duration_cast<seconds>(start2-endsim).count();
	Output::Inst()->kprintf("Simulation run took:\t %llu[s] "
			, duration_cast<seconds>(endsim - start2).count()			
			);

}

/**
 * Stop currently running simulation
 * Stops the active simulation run via setting an atomic boolean.
 */
void AgentDomain::stopSimulation(){
	stop = true;
	Output::Inst()->clearProgressBar();
}

/**
 * Save eEvent data to disk
 * @see EventQueue::saveEEventData
 */
void AgentDomain::saveExternalEvents(std::string filename){
	master.saveExternalEvents(filename);
}
