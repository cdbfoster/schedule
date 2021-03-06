/*
* This file is part of schedule.
*
* schedule is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* schedule is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with schedule. If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2015 Chris Foster
*/

#ifndef SCHEDULE_SCHEDULEFILEIO
#define SCHEDULE_SCHEDULEFILEIO

#include "Schedule.hpp"

namespace Schedule
{
	class ScheduleFileIO
	{
	public:
		static Schedule	Read(std::string const &FileName);
		static bool		Write(Schedule const &Schedule, std::string const &FileName);
	};
}

#endif
