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
#ifndef MASTER_H
#define MASTER_H

#include <vector>
#include <list>
#include <string>

#include"nestene.h"

class Nestene;
class Master
{
	public:
		/*
		  resolution is microseconds pr. turn
		*/
		Master();
		~Master();
		/*
		   height of map
		   width of map
		   resolution is is number of nestenes squared
		   */
		void generateMap(double width, double height, int resolution, 
				double timeResolution, double macroResolution);


		/*
		   Populate the system with a give population size
		   */
		void populateSystem(int listenerSize, int screamerSize, 
				int LUASize, std::string filename);

		void populateSquareSystem(int LUASize, std::string filename);
		void populateSquareListenerSystem(int listenerSize);

		/*
		   Functions to excecute a microStep and macroStep
		   */
		//void microStep();
		void microStep(unsigned long long tmu);
		void macroStep(unsigned long long tmu);
		unsigned long long getNextMicroTmu();
		/*
		   Functions on what to do when receiving events:
		   */
		void receiveEEventPtr(EventQueue::eEvent* eEvent);
		void receiveInitEEventPtr(EventQueue::eEvent* eEvent);

		/*
		   Functions to add events to the masters eventQueue
		   */
		void receiveIEventPtr(EventQueue::iEvent *iEvent);
		void addExternalEventPtr(EventQueue::eEvent *eEvent);

		void printStatus();
		void retrievePopPos(std::list<double> &sylist, std::list<double> &sxlist,
				std::list<double> &lylist, std::list<double> &lxlist,
				std::list<double> &aylist, std::list<double> &axlist);
		void saveExternalEvents(std::string filename);

		void simDone();

	private:
		unsigned long long tmu;

		std::vector<Nestene> nestenes;
		std::vector<Nestene>::iterator itNest;

		//functions for the different phases in a microstep:
		//list to hold events generated each step.
		std::list<EventQueue::eEvent*> stepEvents;

		//first the master will be queuering the eventQueue if 
		//events will be excecuted here.
		void excecuteEvents();

		//then Nestenes will be queuried to check if an Auton will
		//initiate an event.
		void queryNestenes();
		double timeResolution;
		double macroResolution;

		int autonAmount;
		std::string luaFilename;
		double areaX;
		double areaY;

		EventQueue *eventQueue;

		unsigned long long eEventInitAmount;
		unsigned long long externalDistroAmount;
		unsigned long long responseAmount;
};
#endif // MASTER_H
