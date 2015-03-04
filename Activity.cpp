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

#include "Activity.hpp"
#include "Schedule.hpp"

using namespace Schedule;

Activity::Activity() :
	Name("Activity"),
	DesiredLength(Duration(1, 0, 0)),
	Beginning(nullptr)
{
	this->ActivityStartMode = StartMode::FREE;
	this->ActivityLengthMode = LengthMode::FREE;

	this->Owner = nullptr;
}


Activity::Activity(Activity const &Other)
{
	*this = Other;

	if (Other.Beginning != nullptr)
		this->Beginning = new Offset(*Other.Beginning);

	this->Owner = nullptr;
}


Activity::~Activity()
{
	if (this->Beginning != nullptr)
		delete this->Beginning;
}


std::string	Activity::GetName() const					{ return this->Name; }
void		Activity::SetName(std::string const &Name)	{ this->Name = Name; }


Activity::StartMode Activity::GetStartMode() const { return this->ActivityStartMode; }


void Activity::SetStartMode(StartMode Mode)
{
	this->ActivityStartMode = Mode;
	this->UpdateSchedule();
}


Activity::LengthMode Activity::GetLengthMode() const { return this->ActivityLengthMode; }


void Activity::SetLengthMode(LengthMode Mode)
{
	this->ActivityLengthMode = Mode;
	this->UpdateSchedule();
}


Duration	Activity::GetDesiredLength() const		{ return this->DesiredLength; }
Offset		Activity::GetDesiredStartTime() const	{ return this->DesiredStartTime; }
Offset		Activity::GetDesiredEndTime() const		{ return this->DesiredEndTime; }


void Activity::SetDesiredLength(Duration const &Length)
{
	this->DesiredLength = Length;
	this->UpdateSchedule();
}


void Activity::SetDesiredStartTime(Offset const &StartTime)
{
	this->DesiredStartTime = StartTime;
	this->UpdateSchedule();
}


void Activity::SetDesiredEndTime(Offset const &EndTime)
{
	this->DesiredEndTime = EndTime;
	this->UpdateSchedule();
}


Duration	Activity::GetActualLength() const		{ return this->ActualLength; }
Offset		Activity::GetActualEndTime() const		{ return this->ActualEndTime; }


Offset Activity::GetActualStartTime() const
{
	if (this->Beginning != nullptr)
		return *this->Beginning;
	else
		return this->ActualStartTime;
}


Duration	Activity::GetOptimalLength() const		{ return this->OptimalLength; }
Offset		Activity::GetOptimalStartTime() const	{ return this->OptimalStartTime; }
Offset		Activity::GetOptimalEndTime() const		{ return this->OptimalEndTime; }


void Activity::SetActualLength(Duration const &Length)		{ this->ActualLength = Length; }
void Activity::SetActualStartTime(Offset const &StartTime)	{ this->ActualStartTime = StartTime; }
void Activity::SetActualEndTime(Offset const &EndTime)		{ this->ActualEndTime = EndTime; }


void Activity::SetOptimalLength(Duration const &Length)		{ this->OptimalLength = Length; }
void Activity::SetOptimalStartTime(Offset const &StartTime)	{ this->OptimalStartTime = StartTime; }
void Activity::SetOptimalEndTime(Offset const &EndTime)		{ this->OptimalEndTime = EndTime; }


Offset const *Activity::GetBeginning() const { return this->Beginning; }


void Activity::SetBeginning(Offset const &Beginning)
{
	if (this->Beginning != nullptr)
		*this->Beginning = Beginning;
	else
		this->Beginning = new Offset(Beginning);
}


void Activity::ClearBeginning()
{
	if (this->Beginning != nullptr)
		delete this->Beginning;

	this->Beginning = nullptr;
}


void Activity::UpdateSchedule()
{
	if (this->Owner != nullptr)
		this->Owner->Update();
}
