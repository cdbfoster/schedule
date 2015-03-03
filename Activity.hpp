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

#include <list>
#include <string>

#include "Offset.hpp"

namespace Schedule
{
	class Schedule;

	class Activity
	{
	public:
		Activity();
		Activity(Activity const &Other);
		~Activity();

		std::string	GetName() const;
		void		SetName(std::string const &Name);

		enum class StartMode { FREE,
							   FIXED_ABSOLUTE,
							   FIXED_RELATIVE };

		StartMode	GetStartMode() const;
		void		SetStartMode(StartMode Mode);

		enum class LengthMode { FREE,
								FIXED };

		LengthMode	GetLengthMode() const;
		void		SetLengthMode(LengthMode Mode);

		Duration	GetDesiredLength() const;
		Offset		GetDesiredStartTime() const;
		Offset		GetDesiredEndTime() const;

		void SetDesiredLength(Duration const &Length);
		void SetDesiredStartTime(Offset const &StartTime);
		void SetDesiredEndTime(Offset const &EndTime);

		Duration	GetActualLength() const;
		Offset		GetActualStartTime() const;
		Offset		GetActualEndTime() const;

		Duration	GetOptimalLength() const;
		Offset		GetOptimalStartTime() const;
		Offset		GetOptimalEndTime() const;

		Offset const *GetBeginning() const;

	private:
		friend class Schedule;

		void SetActualLength(Duration const &Length);
		void SetActualStartTime(Offset const &StartTime);
		void SetActualEndTime(Offset const &EndTime);

		void SetOptimalLength(Duration const &Length);
		void SetOptimalStartTime(Offset const &StartTime);
		void SetOptimalEndTime(Offset const &EndTime);

		void SetBeginning(Offset const &Beginning);
		void ClearBeginning();

		Schedule *Owner;

	private:
		void UpdateSchedule();

		std::string	Name;

		StartMode	ActivityStartMode;
		LengthMode	ActivityLengthMode;

		Duration	DesiredLength;
		Offset		DesiredStartTime;
		Offset		DesiredEndTime;

		Duration	ActualLength;
		Offset		ActualStartTime;
		Offset		ActualEndTime;

		Duration	OptimalLength;
		Offset		OptimalStartTime;
		Offset		OptimalEndTime;

		Offset	   *Beginning;
	};

	typedef std::list<Activity *> ActivityList;
}
