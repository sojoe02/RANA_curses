----begin_license--
--
--Copyright 	2013 	Søren Vissing Jørgensen.
--			2014	Søren Vissing Jørgensen, Center for Biorobotics, Sydansk Universitet MMMI.  
--
--This file is part of RANA.
--
--RANA is free software: you can redistribute it and/or modify
--it under the terms of the GNU General Public License as published by
--the Free Software Foundation, either version 3 of the License, or
--(at your option) any later version.
--
--RANA is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU General Public License for more details.
--
--You should have received a copy of the GNU General Public License
--along with RANA.  If not, see <http://www.gnu.org/licenses/>.
--
----end_license--

function serializeTbl(val, name, depth)
	--skipnewlines = skipnewlines or false
	depth = depth or 0
	local tbl = string.rep("", depth)
	if name then 
		tbl = tbl .. name .. " = "
		--else tbl = tbl .. "systbl="
	end	
	if type(val) == "table" then
		tbl = tbl .. "{"
		local i = 1
		for k, v in pairs(val) do
			if i ~= 1 then
				tbl = tbl .. ","
			end	
			tbl = tbl .. serializeTbl(v,k, depth +1) 
			i = i + 1;
		end
		tbl = tbl .. string.rep(" ", depth) ..  "}"
	elseif type(val) == "number" then
		tbl = tbl .. tostring(val) 
	elseif type(val) == "string" then
		tbl = tbl .. string.format("%q", val)
	else
		tbl = tbl .. "[datatype not serializable:".. type(val) .. "]"
	end

	return tbl
end

--......................................................................--
-- 			C++ VALID FUNCTION CALLS			--
--......................................................................--
-- PHYSICS FUNCTIONS:							--
-- 									--
-- l_speedOfSound(x, y, orig_x, orig_x), calculates the amount of 	--
-- 	microsteps it takes for sound to get from origin auton to this 	--
--	auton.	--							--
-- l_distance((x, y, origX, orig_x), calculates distance 		--
-- 	from origin(x,y) to x,y						--
-- l_currentTime(), returns the currently active timestep.		--
-- l_getMacroFactor(), returns the macrofactor value			--
-- l_getTimeResolution(), returns the timeresolution [s]		--	
-- 									--
-- 									--
-- 									--
-- 									--
--									--
-- GENERAL FUNCTIONS:							--
-- l_debug(msg), print a string in the output window of the simulator.	--
-- l_generateEventID(), returns a unique ID which can be assigned to	-- 
-- 	an event.							--
--......................................................................--

-- Global variables:
posX = 0
posY = 0
ID = 0
macroFactor = 0
timeResolution = 0

intensityThr = 0
callStrength = 0

--Define the function meta table:
func ={}
-- generic function caller:
function func.execute(name, index, ...)
	return func[name]["f"..index](...)
end
func.soundIntensity = {}
function func.soundIntensity.f1(...)
	setPosX, setPosY = ...
	x = 0
	y = 0
	if setPosX and setPosY then
		x = setPosX-posX
		y = setPosY-posY
	end
	l = math.sqrt(x*x + y*y)/10-1
	return 1/(math.exp(l)+1)
end
function func.soundIntensity.f2(...)
	Dthr, setPosX, setPosY = ...
	if setPosX and setPosY then
		x = setPosX 
		y = setPosY
	else
		x = posX
		y = posY
	end
	l = math.sqrt(x*x + y*y)/ Dthr - 1
	return 1/(math.exp(l) + 1 )
end

-- Init of the lua frog, function called upon initilization of the LUA auton:
function initAuton(x, y, id, macroFactor, timeResolution)
	--load the table and function handling LUA:
	--dofile("LUA_preloaded.lua")
	--l_debug(x.." " .." "..y.." ".. id.." "
	--.. macroFactor .." "
	--.. timeResolution .. "\n")
	posX = x
	posY = y
	ID = id
	macroFactor = macroFactor
	timeResolution = timeResolution
	intensityThr = 0.5
	callStrength = 0.5
	energyLevel = 0.1
end

-- Handling of an external event
-- Will recieve all relevant data, can return a "null" string (string = "null") 
-- if it doesn't want to return an internal event, else it will return. 
-- it can also calculate when it really wants to make a descision on whether 
-- it wants to deal with the event to a later point by returning data for
-- an internal event with an activation time.
-- @param origX originators x location.
-- @param origY originators y location.
-- @param origID originators id.
-- @param origDesc origniators description.
-- @param origTable originiators information table.
-- @return "null" string if no event is to be initiated. or 
function handleExternalEvent(origX, origY, eventID, eventDesc , eventTable)
	activationTime = l_speedOfSound(posX, posY, origX, origX)

	--l_debug("Auton with ID" .. ID .. " received event :\n")

	--l_debug(origX.." ".. origY .." "
	--.. eventID .." " .. eventTable .. " "
	--.. timeResolution .. " ".. activationTime .."\n")

	return 1,2,3,"null"	
end
-- Handling an internal event, will recieve all data from the external event
-- that caused it, can return data for an external event or a 'null' string 
-- for no external event.
-- @param origX originators x location.
-- @param origY originators y location.
-- @param origID originators id.
-- @param origDesc origniators description.
-- @param origTable originiators information table.
function handleInternalEvent(origX, origY, origID, origDesc , origTable)

	if energyLevel > 0.5 then
		calltable = {index = 1, arg1 = 2}

		s_calltable = serializeTbl(calltable) 
		desc = "sound"
		id = l_generateEventID()
		activationTime = l_currenttime()		
		energyLevel = 0

		return s_calltable, desc, id, activationTime
	else
		energyLevel = energyLevel + 0.1 * l_getMacroFactor() * l_getTimeResolution()
		return "null"
	end

end	

--Determine whether or not this Auton will initiate an event.
function initiateEvent()
	if energyLevel > 0.5 then
		calltable = {name = "soundIntensity", index = 2, arg1 = 1}
		s_calltable = serializeTbl(calltable) 
		desc = "sound"
		id = l_generateEventID()
		activationTime = l_currentTime()		
		energyLevel = 0
		return s_calltable, desc, id, activationTime
	else
		energyLevel = energyLevel + 0.1 
		* l_getMacroFactor() * l_getTimeResolution()
		return "1","3","4","null"

	end
end

--
function processFunction(posX, posY, callTable)
	--load the callTable:
	loadstring("ctable="..callTable)()
	--handle the call:
	if ctable.name == "soundIntensity" then
		if ctable.index == 1 then
			return func.execute(ctable.name, ctable.index, posX, posY)
		elseif ctable.index == 2 then
			return func.execute(ctable.name, ctable.index, ctable.arg1, posX, posY)
		end
	end
end
