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

	if (CurrentActivity.GetName() != "Pause")
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
		std::cout << " Pause ";

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


void DisplayHelp()
{
	std::vector<std::string>::const_iterator Argument = Arguments.begin();

	// Skip over the file, quiet, and help arguments
	{
		std::string Next;
		if (Get(Argument, Next) &&
			Next != "list" &&
			Next != "add" &&
			Next != "set" &&
			Next != "move" &&
			Next != "remove" &&
			Next != "begin" &&
			Next != "reset" &&
			Next != "pause" &&
			Next != "-q")
		{
			++Argument;
		}

		if (Compare(Argument, "-q"))
			++Argument;

		if (Compare(Argument, "-h") || Compare(Argument, "--help"))
			++Argument;
	}

	std::string Command;
	if (Get(Argument, Command))
		++Argument;

	std::vector<std::pair<std::string, std::string>> Usages = {
		{"list",	"list [Activity]\n\n"
		 "List the schedule.  Optionally, list only Activity."},
		{"add",		"add [-b Before] [-n Name] [-fs (f | a | r)] [-s Start] [-fl (f | a)] [-l Length]\n\n"
		 "Add a new activity to the schedule.\n"
		 " -b    Insert the new activity before Before.\n\n"
		 " -n    Set the new activity's name to Name.\n\n"
		 " -fs   Specify the new activity's start mode:\n"
		 "         f - Free (default)\n"
		 "         a - Fixed-Absolute\n"
		 "         r - Fixed-Relative\n\n"
		 " -s    Set the new activity's desired start time to Start.\n"
		 "       Times are specified in the format hh:mm:ss\n\n"
		 " -fl   Specify the new activity's length mode:\n"
		 "         f - Free (default)\n"
		 "         a - Fixed\n\n"
		 " -l    Set the new activity's desired length to Length.\n"
		 "       Times are specified in the format hh:mm:ss"},
		{"set",		"set -l Length\n\n"
		 "Set the schedule's length:\n"
		 " -l    Set the schedule's length to Length.\n"
		 "       Times are specified in the format hh:mm:ss\n\n"
		 " schedule set Activity [-n Name] [-fs (f | a | r)] [-s Start] [-fl (f | a)] [-l Length])\n\n"
		 "Set Activity's details:\n"
		 " -n    Set the activity's name to Name.\n\n"
		 " -fs   Specify the activity's start mode:\n"
		 "         f - Free\n"
		 "         a - Fixed-Absolute\n"
		 "         r - Fixed-Relative\n\n"
		 " -s    Set the activity's desired start time to Start.\n"
		 "       Times are specified in the format hh:mm:ss\n\n"
		 " -fl   Specify the activity's length mode:\n"
		 "         f - Free\n"
		 "         a - Fixed\n\n"
		 " -l    Set the activity's desired length to Length.\n"
		 "       Times are specified in the format hh:mm:ss"},
		{"move",	"move Activity Before\n\n"
		 "Move Activity before Before.  If Activity is equal to Before, Activity is moved\n"
		 "to the end of the schedule."},
		{"remove",	"remove Activity\n\n"
		 "Delete Activity from the schedule."},
		{"begin",	"begin [Activity [BeginTime]]\n\n"
		 "Begin an activity.  If no activity is specified, the next unbegun activity is\n"
		 "begun, at the current time of day.  The beginning time can be specified only if\n"
		 "the activity to begin is also specified.  This also ends an active pause in the\n"
		 "schedule, if there is one."},
		{"reset",	"reset [Activity]\n\n"
		 "Reset all beginnings in the schedule, and delete all pauses.  Activities that were\n"
		 "split by pauses are joined again.  If Activity is specified, reset only Activity."},
		{"pause",	"pause [PauseTime]\n\n"
		 "Insert a pause at the current time of day or at the specified pause time.  Activities\n"
		 "intersected by the pause are split such that the parts equal the unpaused whole."}
	};

	if (Command == "" ||
	   (Command != "list" &&
		Command != "add" &&
		Command != "set" &&
		Command != "move" &&
		Command != "remove" &&
		Command != "begin" &&
		Command != "reset" &&
		Command != "pause"))
	{
		std::cout << "Usage:\n"
					 " schedule [File] [-q] [Command] [Options...]\n\n"
					 "A small daily scheduling program that scales activities according to the amount of\n"
					 "time available in the schedule.\n"
					 " File       The schedule file to use.  If omitted, default.sch is used.\n\n"
					 " -q         Quiet mode.\n\n"
					 " Command    The command to execute.  Commands are:\n"
					 "              list (default, if omitted)\n"
					 "              add\n"
					 "              set\n"
					 "              move\n"
					 "              remove\n"
					 "              begin\n"
					 "              reset\n"
					 "              pause\n\n"
					 "Use \"schedule --help Command\" for more info on Command." << std::endl;
	}
	else
	{
		for (auto &Usage : Usages)
		{
			if (Command == Usage.first)
			{
				std::cout << "Usage:\n"
							 " schedule " << Usage.second << std::endl;

				break;
			}
		}
	}
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
			Next != "set" &&
			Next != "move" &&
			Next != "remove" &&
			Next != "begin" &&
			Next != "reset" &&
			Next != "pause" &&
			Next != "-q" &&
			Next != "-h" &&
			Next != "--help")
		{
			ScheduleFileName = Next;

			++Argument;
		}
	}

	bool Quiet = false;
	if (Compare(Argument, "-q"))
	{
		Quiet = true;
		++Argument;
	}

	if (Compare(Argument, "-h") || Compare(Argument, "--help"))
	{
		DisplayHelp();
		return 0;
	}


	Schedule::Schedule CurrentSchedule = Schedule::ScheduleFileIO::Read(ScheduleFileName);


	std::string Command;
	if (Get(Argument, Command))
		++Argument;


	if ((Command == "list" || Command == "") && !Quiet)
	{
		std::string Next;
		if (Get(Argument, Next))
		{
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

				if (!Quiet)
					DisplaySchedule(CurrentSchedule);

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
					Next != "-fs" &&
					Next != "-s" &&
					Next != "-fl" &&
					Next != "-l")
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


	else if (Command == "begin")
	{
		// Find the number of the activity to begin, specified or otherwise
		unsigned int BeginNumber = 0;
		if (!Get(Argument, BeginNumber))
		{
			unsigned int Index = CurrentSchedule.size();
			for (Schedule::Schedule::const_reverse_iterator ActivityIterator = CurrentSchedule.rbegin();
															ActivityIterator != CurrentSchedule.rend();
															++ActivityIterator, Index--)
			{
				Schedule::Activity const * const CurrentActivity = *ActivityIterator;
				if (CurrentActivity->GetBeginning() == nullptr && CurrentActivity->GetName() != "Pause")
					BeginNumber = Index;
				else
					break;
			}
		}
		else
			++Argument;

		if (BeginNumber == 0 || BeginNumber > CurrentSchedule.size())
		{
			std::cerr << "Activity number out of range." << std::endl;
			return 2;
		}


		// Find the activity to begin, and the active pause if there is one
		Schedule::Activity					   *BeginActivity;
		Schedule::Activity					   *ActivePause = nullptr;
		Schedule::Schedule::reverse_iterator	ActivePauseIterator = CurrentSchedule.rend();
		{
			unsigned int Index = CurrentSchedule.size();
			Schedule::Schedule::reverse_iterator ActivityIterator;
			for (ActivityIterator = CurrentSchedule.rbegin();
				 ActivityIterator != CurrentSchedule.rend();
				 ++ActivityIterator, Index--)
			{
				if (Index == BeginNumber)
				{
					BeginActivity = *ActivityIterator;

					if (BeginActivity->GetName() == "Pause")
					{
						std::cerr << "Cannot begin a pause." << std::endl;
						return 2;
					}

					break;
				}
			}

			for (; ActivityIterator != CurrentSchedule.rend();
				   ++ActivityIterator)
			{
				Schedule::Activity * const CurrentActivity = *ActivityIterator;

				if (CurrentActivity->GetName() == "Pause")
				{
					if (CurrentActivity->GetLengthMode() != Schedule::Activity::LengthMode::FIXED)
					{
						ActivePause = CurrentActivity;
						ActivePauseIterator = ActivityIterator;
					}

					// Only check the first pause we come to
					break;
				}
			}
		}


		// Get the beginning time, specified or otherwise
		Schedule::Offset BeginOffset;
		{
			std::string Next;
			if (!Get(Argument, Next))
				BeginOffset = Schedule::Offset::GetLocalTimeOfDay();
			else
				BeginOffset = OffsetTranslator::ToOffset(Next);
		}


		// Close the active pause if there is one
		if (ActivePause != nullptr)
		{
			Schedule::Duration const PauseLength = BeginOffset - ActivePause->GetActualStartTime();

			if (PauseLength.IsNegative())
			{
				std::cerr << "The beginning time is earlier than the active pause." << std::endl;
				return 2;
			}

			ActivePause->SetDesiredLength(PauseLength);
			ActivePause->SetLengthMode(Schedule::Activity::LengthMode::FIXED);
		}


		// Begin the activity and write it to the schedule
		CurrentSchedule.BeginActivity(*BeginActivity, BeginOffset);
		Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}


	else if (Command == "reset")
	{
		unsigned int ResetNumber = 0;
		if (!Get(Argument, ResetNumber))
		{
			Schedule::Schedule::iterator Previous = CurrentSchedule.end();

			for (Schedule::Schedule::iterator ActivityIterator = CurrentSchedule.begin();
											  ActivityIterator != CurrentSchedule.end();
											  Previous = ActivityIterator++)
			{
				if ((*ActivityIterator)->GetName() == "Pause")
				{
					// Delete the pause
					{
						Schedule::Activity * const Pause = *ActivityIterator;
						ActivityIterator = CurrentSchedule.erase(ActivityIterator);
						delete Pause;
					}

					// Delete the second half of an activity that was split by the pause
					if (Previous != CurrentSchedule.end() &&
						ActivityIterator != CurrentSchedule.end() &&
						(*Previous)->GetName() == (*ActivityIterator)->GetName())
					{
						Schedule::Activity * const Split = *ActivityIterator;
						ActivityIterator = CurrentSchedule.erase(ActivityIterator);
						delete Split;
					}
				}
				else
					CurrentSchedule.ClearBeginning(**ActivityIterator);
			}

			Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);
		}
		else
		{
			if (ResetNumber == 0 || ResetNumber > CurrentSchedule.size())
			{
				std::cerr << "Activity number out of range." << std::endl;
				return 2;
			}

			unsigned int Index = 1;
			for (auto Activity : CurrentSchedule)
			{
				if (Index == ResetNumber)
				{
					CurrentSchedule.ClearBeginning(*Activity);
					break;
				}

				Index++;
			}

			Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);
		}

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}


	else if (Command == "pause")
	{
		if (CurrentSchedule.size() == 0)
		{
			std::cerr << "The schedule is empty.  Cannot pause." << std::endl;
			return 2;
		}


		// Get the pause time and check for sanity
		Schedule::Offset PauseTime;
		{
			std::string Next;
			if (!Get(Argument, Next))
				PauseTime = Schedule::Offset::GetLocalTimeOfDay();
			else
				PauseTime = OffsetTranslator::ToOffset(Next);

			if (PauseTime < CurrentSchedule.front()->GetActualStartTime() ||
				PauseTime >= CurrentSchedule.back()->GetActualStartTime() + CurrentSchedule.back()->GetActualLength())
			{
				std::cerr << "The pause time is outside of the schedule.  Cannot pause." << std::endl;
				return 2;
			}

			for (auto Activity : CurrentSchedule)
			{
				if (Activity->GetName() == "Pause" &&
					Activity->GetLengthMode() != Schedule::Activity::LengthMode::FIXED)
				{
					std::cerr << "There is already an active pause in the schedule.  Cannot pause." << std::endl;
					return 2;
				}
			}
		}


		// Get the activity to split and the one after it
		Schedule::Activity					   *BeforeActivity = nullptr;
		Schedule::Schedule::reverse_iterator	BeforeActivityIterator = CurrentSchedule.rend();
		Schedule::Schedule::iterator			NextActivityIterator = CurrentSchedule.end();
		{
			for (Schedule::Schedule::reverse_iterator ActivityIterator = CurrentSchedule.rbegin();
													  ActivityIterator != CurrentSchedule.rend();
													  ++ActivityIterator)
			{
				if ((*ActivityIterator)->GetActualStartTime() <= PauseTime)
				{
					BeforeActivity = *ActivityIterator;
					BeforeActivityIterator = ActivityIterator;
					break;
				}
			}

			NextActivityIterator = BeforeActivityIterator.base();
		}


		// Split the activity and insert the pause
		{
			std::string const			BeforeName = BeforeActivity->GetName();
			bool const					BeforeFixedLength = (BeforeActivity->GetLengthMode() == Schedule::Activity::LengthMode::FIXED);
			Schedule::Duration const	BeforeLength = PauseTime - BeforeActivity->GetActualStartTime();
			Schedule::Duration const	BeforeDesiredLength = BeforeActivity->GetDesiredLength();
			Schedule::Duration const	AfterLength = BeforeActivity->GetActualStartTime() + BeforeActivity->GetActualLength() - PauseTime;

			if (BeforeLength.IsZero())
			{
				++BeforeActivityIterator;
				CurrentSchedule.erase(BeforeActivityIterator.base());
				delete BeforeActivity;
			}
			else if (BeforeActivity->GetStartMode() == Schedule::Activity::StartMode::FREE &&
					 BeforeActivity->GetBeginning() == nullptr)
			{
					BeforeActivity->SetDesiredStartTime(BeforeActivity->GetActualStartTime());
					BeforeActivity->SetStartMode(Schedule::Activity::StartMode::FIXED_ABSOLUTE);
			}

			{
				Schedule::Activity * const PauseActivity = new Schedule::Activity;
				PauseActivity->SetName("Pause");
				PauseActivity->SetDesiredStartTime(PauseTime);
				PauseActivity->SetStartMode(Schedule::Activity::StartMode::FIXED_ABSOLUTE);
				PauseActivity->SetDesiredLength(Schedule::Duration());

				CurrentSchedule.insert(NextActivityIterator, PauseActivity);
			}

			if (!AfterLength.IsZero())
			{
				Schedule::Activity * const AfterActivity = new Schedule::Activity;
				AfterActivity->SetName(BeforeName);

				if (!BeforeFixedLength)
				{
					float const RemainingTimeScale = (float)AfterLength.GetTotalSeconds() / (BeforeLength + AfterLength).GetTotalSeconds();
					AfterActivity->SetDesiredLength(BeforeDesiredLength * RemainingTimeScale);
				}
				else
				{
					AfterActivity->SetDesiredLength(AfterLength);
					AfterActivity->SetLengthMode(Schedule::Activity::LengthMode::FIXED);
				}

				CurrentSchedule.insert(NextActivityIterator, AfterActivity);
			}

			Schedule::ScheduleFileIO::Write(CurrentSchedule, ScheduleFileName);
		}

		if (!Quiet)
			DisplaySchedule(CurrentSchedule);
	}
	else
		DisplayHelp();

	return 0;
}
