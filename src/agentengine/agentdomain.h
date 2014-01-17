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
#ifndef AGENTDOMAIN_H
#define AGENTDOMAIN_H

#include<iostream>
#include<mutex>
#include<atomic>
#include "agents/master.h"


class AgentDomain
{
	public:
		AgentDomain();
		~AgentDomain();

		/*
		   function to intepret the relevant LUA functions for the AgentEngine.
		   */
		void interpret(std::string function);

		void generateEnvironment(double width, double height, int resolution,
				int listenerSize, int screamerSize, int LUASize,
				double timeResolution, int macroFactor, std::string filename);

		void generateSquaredEnvironment(double width, double height, int resolution,int LUASize,double timeResolution, int macroFactor, std::string filename);

		void generateSquaredListenerEnvironment(double width, double height, int resolution,int LUASize,double timeResolution, int macroFactor);


		void retrievePopPos(std::list<double> &sylist, std::list<double> &sxlist,
				std::list<double> &lylist, std::list<double> &lxlist,
				std::list<double> &aylist, std::list<double> &axlist, double &width, double &height);

		void runSimulation(int time);

		bool checkEnvPresence();
		void stopSimulation();
		void saveExternalEvents(std::string filename);
		void updateStatus();

	private:		
		bool mapGenerated;
		Master master;
		double timeResolution;
		double macroResolution;
		int macroFactor;
		int mapWidth, mapHeight;
		unsigned long long iterations;
		unsigned long long i;


		//Atomic thread controllers:
		std::atomic_bool stop;
		std::mutex stopMutex;

};

#endif // AGENTDOMAIN_H
