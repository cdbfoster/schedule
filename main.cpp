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

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Activity.hpp"
#include "Offset.hpp"
#include "Schedule.hpp"
#include "ScheduleFileIO.hpp"

void DisplayHelp()
{
	std::cout << "You have no idea what you're doing, do you?" << std::endl;
}


struct OffsetTranslator
{
	static Schedule::Offset ToOffset(std::string const &String)
	{
		std::istringstream Stream(String);
		std::list<long> Values;
		std::string Item;

		int Size;
		for (Size = 0; Size < 3 && std::getline(Stream, Item, ':'); Size++)
		{
			long Value;
			std::istringstream(Item) >> Value;
			Values.push_back(Value);
		}

		// Pad the beginning so that there is a value for each denomination
		for (int a = Size; a < 3; a++)
			Values.push_front(0);

		std::list<long>::const_iterator Position = Values.begin();

		long const Hours	= *Position++;
		long const Minutes	= *Position++;
		long const Seconds	= *Position;

		return Schedule::Offset(Hours, Minutes, Seconds);
	}

	static std::string ToString(Schedule::Offset const &Offset)
	{
		std::ostringstream Stream;

		if (long const Value = Offset.GetHours())
		{
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value << ":";
		}

		if (long const Value = Offset.GetMinutes())
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value << ":";
		else
			Stream << "00:";

		if (long const Value = Offset.GetSeconds())
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value;
		else
			Stream << "00";

		return Stream.str();
	}
};


std::string FixedWidthString(std::string const &Input, unsigned int Width, std::ios_base &(& Align)(std::ios_base &) = std::left,
							 char Pad = ' ', std::string const &Continuation = "...")
{
	if (Width < Continuation.length())
		Width = Continuation.length();

	std::ostringstream Stream;

	Stream << Align << std::setw(Width) << std::setfill(Pad) << Input;

	std::string Result = Stream.str();

	if (Result.length() > Width)
	{
		Result.erase(Width - Continuation.length(), Result.length() - Width + Continuation.length());
		Result += Continuation;
	}

	return Result;
}


void DisplayHeader(unsigned int NameWidth)
{
	std::cout << FixedWidthString("Index", 5) << " | " <<
				 FixedWidthString("Fixed", 5) << " | " <<
				 FixedWidthString("Start", 8) << " | " <<
				 FixedWidthString("Activity Name", NameWidth) << " | " <<
				 FixedWidthString("Length", 8) << " | " <<
				 FixedWidthString("Desired Start", 13) << " | " <<
				 FixedWidthString("Desired Length", 14) << std::endl;
}


void DisplayActivity(Schedule::Activity &CurrentActivity, unsigned int Index, unsigned int NameWidth)
{
	{
		std::ostringstream Stream;

		Stream << Index;

		std::cout << FixedWidthString(Stream.str(), 5, std::internal) << "   ";
	}

	if (CurrentActivity.GetName() != "Pause" && CurrentActivity.GetName() != "Stop")
	{
		{
			std::string FixedString = std::string(CurrentActivity.GetStartMode() == Schedule::Activity::StartMode::FREE ? "-- " :
												 (CurrentActivity.GetStartMode() == Schedule::Activity::StartMode::FIXED_ABSOLUTE ? "FA " : "FR ")) +
												 (CurrentActivity.GetLengthMode() == Schedule::Activity::LengthMode::FREE ? "--" : "FA");

			if (CurrentActivity.GetBeginning() != nullptr)
				FixedString.replace(0, 1, "B");

			std::cout << FixedWidthString(FixedString, 5) << "   ";
		}

		std::cout << FixedWidthString(OffsetTranslator::ToString(CurrentActivity.GetActualStartTime()), 8, std::right) << "   " <<
					 FixedWidthString(CurrentActivity.GetName(), NameWidth) << "   " <<
					 FixedWidthString(OffsetTranslator::ToString(CurrentActivity.GetActualLength()), 8, std::right) << "   " <<
					 FixedWidthString(std::string(CurrentActivity.GetStartMode() == Schedule::Activity::StartMode::FIXED_RELATIVE ? "R " : "") +OffsetTranslator::ToString(CurrentActivity.GetDesiredStartTime()), 13, std::right) << "   " <<
					 FixedWidthString(OffsetTranslator::ToString(CurrentActivity.GetDesiredLength()), 14, std::right) << std::endl;
	}
	else
	{
		std::cout << " " << CurrentActivity.GetName() << " ";

		if (CurrentActivity.GetLengthMode() != Schedule::Activity::LengthMode::FIXED)
			std::cout << "initiated at " << OffsetTranslator::ToString(CurrentActivity.GetActualStartTime()) << std::endl;
		else
			std::cout << "from " << OffsetTranslator::ToString(CurrentActivity.GetActualStartTime()) << " to " <<
									OffsetTranslator::ToString(CurrentActivity.GetActualStartTime() + CurrentActivity.GetActualLength()) << " (Duration: " <<
									OffsetTranslator::ToString(CurrentActivity.GetActualLength()) << ")" << std::endl;
	}
}


void DisplaySchedule(Schedule::Schedule const &CurrentSchedule)
{
	std::cout << "Activies: " << CurrentSchedule.size() << std::endl;

	if (CurrentSchedule.size() == 0)
		return;

	unsigned int LongestName = 0;
	for (Schedule::Schedule::const_iterator	ActivityIterator = CurrentSchedule.begin();
											ActivityIterator != CurrentSchedule.end();
											++ActivityIterator)
	{
		if ((*ActivityIterator)->GetName().length() > LongestName)
			LongestName = (*ActivityIterator)->GetName().length();
	}

	LongestName = (LongestName > 30 ? 30 : (LongestName < 13 ? 13 : LongestName));

	DisplayHeader(LongestName);

	unsigned int Index = 1;

	for (Schedule::Schedule::const_iterator	ActivityIterator = CurrentSchedule.begin();
											ActivityIterator != CurrentSchedule.end();
											++ActivityIterator, Index++)
	{
		DisplayActivity(**ActivityIterator, Index, LongestName);
	}
}


int main(int argc, char **argv)
{
	std::vector<std::string> const Arguments(argv + 1, argv + argc);


	Schedule::Schedule const Test = Schedule::ScheduleFileIO::Read("test.sch");

	DisplaySchedule(Test);

	return 0;
}
