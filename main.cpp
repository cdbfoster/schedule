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


unsigned int VerifyNameWidth(unsigned int NameWidth)
{
	return (NameWidth > 30 ? 30 : (NameWidth < 13 ? 13 : NameWidth));
}


void DisplayHeader(unsigned int NameWidth)
{
	NameWidth = VerifyNameWidth(NameWidth);

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
	NameWidth = VerifyNameWidth(NameWidth);

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
	std::cout << "Length: " << OffsetTranslator::ToString(CurrentSchedule.GetLength()) << " | Activities: " << CurrentSchedule.size() << std::endl;

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

	LongestName = VerifyNameWidth(LongestName);

	DisplayHeader(LongestName);

	unsigned int Index = 1;

	for (Schedule::Schedule::const_iterator	ActivityIterator = CurrentSchedule.begin();
											ActivityIterator != CurrentSchedule.end();
											++ActivityIterator, Index++)
	{
		DisplayActivity(**ActivityIterator, Index, LongestName);
	}
}


std::vector<std::string> Arguments;


template <typename T>
bool Get(std::vector<std::string>::const_iterator const &Argument, T &Out)
{
	if (Argument == Arguments.end())
		return false;

	std::string const	ArgumentString = *Argument;
	std::istringstream	Stream(ArgumentString);

	Stream >> Out;

	return !Stream.fail();
}


bool Get(std::vector<std::string>::const_iterator const &Argument, std::string &Out)
{
	if (Argument == Arguments.end())
		return false;

	Out = *Argument;
	return true;
}


template <typename T>
bool Compare(std::vector<std::string>::const_iterator &Argument, T const &Value)
{
	if (Argument == Arguments.end())
		return false;

	T Out;
	if (!Get(Argument, Out))
		return false;

	return Value == Out;
}


bool Compare(std::vector<std::string>::const_iterator &Argument, char const *Value)
{
	if (Argument == Arguments.end())
		return false;

	std::string Out;
	if (!Get(Argument, Out))
		return false;

	return std::string(Value) == Out;
}


int main(int argc, char **argv)
{
	Arguments.insert(Arguments.end(), argv + 1, argv + argc);
	std::vector<std::string>::const_iterator Argument = Arguments.begin();

	std::string ScheduleFileName = "default.sch";
	{
		std::string Next;
		if (Get(Argument, Next) &&
			Next != "list" &&
			Next != "add" &&
			Next != "move" &&
			Next != "remove" &&
			Next != "set" &&
			Next != "begin" &&
			Next != "reset" &&
			Next != "pause" &&
			Next != "stop" &&
			Next != "-q")
		{
			ScheduleFileName = Next;

			++Argument;
		}
	}

	Schedule::Schedule CurrentSchedule = Schedule::ScheduleFileIO::Read(ScheduleFileName);

	bool Quiet = false;
	if (Compare(Argument, "-q"))
	{
		Quiet = true;
		++Argument;
	}

	std::string Command;
	if (Get(Argument, Command))
		++Argument;


	if ((Command == "list" || Command == "") && !Quiet)
	{
		std::string Next;
		if (Get(Argument, Next))
		{
			++Argument;

			if (Next != "-a")
			{
				DisplayHelp();
				return 1;
			}

			unsigned int ActivityNumber;
			if (!Get(Argument, ActivityNumber))
			{
				DisplayHelp();
				return 1;
			}

			if (ActivityNumber == 0 || ActivityNumber > CurrentSchedule.size())
			{
				std::cerr << "Activity number out of range." << std::endl;
				return 2;
			}

			{
				unsigned int Index = 1;
				for (auto Activity : CurrentSchedule)
				{
					if (Index == ActivityNumber)
					{
						unsigned int const NameWidth = Activity->GetName().size();

						DisplayHeader(NameWidth);
						DisplayActivity(*Activity, Index, NameWidth);
						break;
					}

					Index++;
				}

				return 0;
			}
		}
		else
			DisplaySchedule(CurrentSchedule);
	}


	else if (Command == "add" || Command == "set")
	{
		// Get the index of the activity to modify if were're setting and not adding
		unsigned int SelectedIndex;
		if (Command == "set")
		{
			if (Compare(Argument, "-l"))
			{
				++Argument;

				std::string Next;
				if (!Get(Argument, Next))
				{
					DisplayHelp();
					return 1;
				}

				CurrentSchedule.SetLength(OffsetTranslator::ToOffset(Next));
				Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);

				return 0;
			}
			else
			{
				if (!Get(Argument, SelectedIndex))
				{
					DisplayHelp();
					return 1;
				}
				else
					++Argument;

				if (SelectedIndex == 0 || SelectedIndex > CurrentSchedule.size())
				{
					std::cerr << "Activity number out of range." << std::endl;
					return 2;
				}
			}
		}


		// Read all of the following arguments into a map
		std::map<std::string, std::string> ArgumentMap;
		{
			std::string Next;
			while (Get(Argument, Next))
			{
				if ((Command == "add" && Next != "-b") &&
					Next != "-n" &&
					Next != "-l" &&
					Next != "-fs" &&
					Next != "-fl" &&
					Next != "-s" &&
					Next != "-q")
				{
					DisplayHelp();
					return 1;
				}

				if (ArgumentMap.find(Next) != ArgumentMap.end())
				{
					std::cerr << "Duplicate arguments: " << Next << std::endl;
					return 2;
				}
				else
					++Argument;

				std::string Value;
				if (!Get(Argument, Value))
				{
					DisplayHelp();
					return 1;
				}
				else
					++Argument;

				ArgumentMap.insert(std::make_pair(Next, Value));
			}
		}


		// Create or find the activity to modifiy
		Schedule::Activity *CurrentActivity;
		{
			if (Command == "add")
				CurrentActivity = new Schedule::Activity;
			else
			{
				unsigned int Index = 1;
				for (auto Activity : CurrentSchedule)
				{
					if (Index == SelectedIndex)
					{
						CurrentActivity = Activity;
						break;
					}

					Index++;
				}
			}
		}


		// Go through the arguments and make the requested changes
		{
			Schedule::Schedule::iterator BeforeActivity = CurrentSchedule.end();

			for (auto &Pair : ArgumentMap)
			{
				if (Pair.first == "-b")
				{
					unsigned int BeforeNumber;
					if ((std::istringstream(Pair.second) >> BeforeNumber).fail())
					{
						DisplayHelp();
						delete CurrentActivity;
						return 1;
					}

					if (BeforeNumber == 0 || BeforeNumber > CurrentSchedule.size())
					{
						std::cerr << "Activity number out of range." << std::endl;
						return 2;
					}

					unsigned int Index = 1;
					for (Schedule::Schedule::iterator ActivityIterator = CurrentSchedule.begin();
													  ActivityIterator != CurrentSchedule.end();
													  ++ActivityIterator, Index++)
					{
						if (Index == BeforeNumber)
						{
							BeforeActivity = ActivityIterator;
							break;
						}
					}
				}
				else if (Pair.first == "-n")
					CurrentActivity->SetName(Pair.second);
				else if (Pair.first == "-fs")
				{
					if (Pair.second != "f" && Pair.second != "a" && Pair.second != "r")
					{
						DisplayHelp();

						if (Command == "add")
							delete CurrentActivity;

						return 1;
					}

					CurrentActivity->SetStartMode(Pair.second == "f" ? Schedule::Activity::StartMode::FREE :
												 (Pair.second == "a" ? Schedule::Activity::StartMode::FIXED_ABSOLUTE :
																	   Schedule::Activity::StartMode::FIXED_RELATIVE));
				}
				else if (Pair.first == "-s")
					CurrentActivity->SetDesiredStartTime(OffsetTranslator::ToOffset(Pair.second));
				else if (Pair.first == "-fl")
				{
					if (Pair.second != "f" && Pair.second != "a")
					{
						DisplayHelp();

						if (Command == "add")
							delete CurrentActivity;

						return 1;
					}

					CurrentActivity->SetLengthMode(Pair.second == "f" ? Schedule::Activity::LengthMode::FREE : Schedule::Activity::LengthMode::FIXED);
				}
				else if (Pair.first == "-l")
					CurrentActivity->SetDesiredLength(OffsetTranslator::ToOffset(Pair.second));
			}

			if (Command == "add")
			{
				if (BeforeActivity == CurrentSchedule.end())
					CurrentSchedule.push_back(CurrentActivity);
				else
					CurrentSchedule.insert(BeforeActivity, CurrentActivity);
			}
		}


		// Write the schedule
		Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}


	else if (Command == "move")
	{
		unsigned int MoveNumber;
		unsigned int BeforeNumber;

		if (!Get(Argument, MoveNumber) ||
			!Get(Argument + 1, BeforeNumber))
		{
			DisplayHelp();
			return 1;
		}

		if (MoveNumber == 0 || MoveNumber > CurrentSchedule.size() ||
			BeforeNumber == 0 || BeforeNumber > CurrentSchedule.size())
		{
			std::cerr << "Activity number out of range." << std::endl;
			return 2;
		}

		Schedule::Schedule::iterator MoveActivityIterator	= CurrentSchedule.end();
		Schedule::Schedule::iterator BeforeActivityIterator	= CurrentSchedule.end();

		unsigned int Index = 1;
		for (Schedule::Schedule::iterator ActivityIterator = CurrentSchedule.begin();
										  ActivityIterator != CurrentSchedule.end();
										  ++ActivityIterator, Index++)
		{
			if (Index == MoveNumber)
				MoveActivityIterator = ActivityIterator;
			if (Index == BeforeNumber)
				BeforeActivityIterator = ActivityIterator;

			if (MoveActivityIterator != CurrentSchedule.end() && BeforeActivityIterator != CurrentSchedule.end())
				break;
		}

		Schedule::Activity * const MoveActivity = *MoveActivityIterator;
		CurrentSchedule.erase(MoveActivityIterator);

		if (MoveNumber == BeforeNumber)
			CurrentSchedule.push_back(MoveActivity);
		else
			CurrentSchedule.insert(BeforeActivityIterator, MoveActivity);

		Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}


	else if (Command == "remove")
	{
		unsigned int RemoveNumber;
		if (!Get(Argument, RemoveNumber))
		{
			DisplayHelp();
			return 1;
		}

		if (RemoveNumber == 0 || RemoveNumber > CurrentSchedule.size())
		{
			std::cerr << "Activity number out of range." << std::endl;
			return 2;
		}

		unsigned int Index = 1;
		for (Schedule::Schedule::iterator ActivityIterator = CurrentSchedule.begin();
										  ActivityIterator != CurrentSchedule.end();
										  ++ActivityIterator, Index++)
		{
			if (Index == RemoveNumber)
			{
				Schedule::Activity * const RemoveActivity = *ActivityIterator;

				CurrentSchedule.erase(ActivityIterator);
				delete RemoveActivity;

				break;
			}
		}

		Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}

	return 0;
}
