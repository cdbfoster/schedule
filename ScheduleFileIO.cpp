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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Activity.hpp"
#include "Offset.hpp"
#include "ScheduleFileIO.hpp"

using namespace Schedule;

class OffsetTranslator
{
public:
	typedef std::string	internal_type;
	typedef Offset		external_type;

	boost::optional<external_type> get_value(internal_type const &v)
	{
		std::istringstream Stream(v);
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

		return Offset(Hours, Minutes, Seconds);
	}

	boost::optional<internal_type> put_value(external_type const &v)
	{
		std::ostringstream Stream;

		if (long const Value = v.GetHours())
		{
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value << ":";
		}

		if (long const Value = v.GetMinutes())
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value << ":";
		else
			Stream << "00:";

		if (long const Value = v.GetSeconds())
			Stream << (std::abs(Value) < 10 ? "0" : "") << Value;
		else
			Stream << "00";

		return Stream.str();
	}
};


Schedule::Schedule ScheduleFileIO::Read(std::string const &FileName)
{
	OffsetTranslator Translator;

	boost::property_tree::ptree Root;

	Schedule Staging;

	try
	{
		boost::property_tree::read_xml(FileName, Root);

		boost::property_tree::ptree ScheduleNode = Root.get_child("Schedule");

		if (boost::optional<Duration> Length = ScheduleNode.get_optional<Duration>("Length", Translator))
		{
			Staging.SetLength(*Length);
		}

		for (boost::property_tree::ptree::const_iterator Child = ScheduleNode.begin(); Child != ScheduleNode.end(); ++Child)
		{
			if (Child->first == "Activity")
			{
				Activity *NewActivity = new Activity;

				if (boost::optional<std::string> Name = Child->second.get_optional<std::string>("Name"))
					NewActivity->SetName(*Name);

				if (boost::optional<std::string> StartMode = Child->second.get_optional<std::string>("StartMode"))
					NewActivity->SetStartMode(*StartMode == "Fixed-Absolute" ? Activity::StartMode::FIXED_ABSOLUTE :
											 (*StartMode == "Fixed-Relative" ? Activity::StartMode::FIXED_RELATIVE :
																			   Activity::StartMode::FREE));

				if (boost::optional<Offset> Start = Child->second.get_optional<Offset>("Start", Translator))
					NewActivity->SetDesiredStartTime(*Start);

				if (boost::optional<std::string> LengthMode = Child->second.get_optional<std::string>("LengthMode"))
					NewActivity->SetLengthMode(*LengthMode == "Fixed" ? Activity::LengthMode::FIXED :
																		Activity::LengthMode::FREE);

				if (boost::optional<Duration> Length = Child->second.get_optional<Duration>("Length", Translator))
					NewActivity->SetDesiredLength(*Length);

				Staging.push_back(NewActivity);

				if (boost::optional<Offset> Beginning = Child->second.get_optional<Offset>("Beginning", Translator))
					Staging.BeginActivity(*NewActivity, *Beginning);
			}
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl;

		return Schedule(Duration());
	}

	return Staging;
}


bool ScheduleFileIO::Write(Schedule const &Schedule, std::string const &FileName)
{
	OffsetTranslator Translator;

	boost::property_tree::ptree Root;

	// Set version and length
	boost::property_tree::ptree ScheduleNode;
	ScheduleNode.put("<xmlattr>.version", "1.0");
	ScheduleNode.put("Length", Schedule.GetLength(), Translator);

	// Write each activity
	for (Schedule::Schedule::const_iterator	ActivityIterator = Schedule.begin();
											ActivityIterator != Schedule.end();
											++ActivityIterator)
	{
		Activity const &CurrentActivity = **ActivityIterator;

		boost::property_tree::ptree ActivityNode;

		if (!CurrentActivity.GetName().empty())
			ActivityNode.put("Name", CurrentActivity.GetName());

		if (CurrentActivity.GetStartMode() != Activity::StartMode::FREE)
			ActivityNode.put("StartMode", (CurrentActivity.GetStartMode() == Activity::StartMode::FIXED_ABSOLUTE ?
											   "Fixed-Absolute" : "Fixed-Relative"));

		if (!CurrentActivity.GetDesiredStartTime().IsZero())
			ActivityNode.put("Start", CurrentActivity.GetDesiredStartTime(), Translator);

		if (CurrentActivity.GetLengthMode() != Activity::LengthMode::FREE)
			ActivityNode.put("LengthMode", "Fixed");

		ActivityNode.put("Length", CurrentActivity.GetDesiredLength(), Translator);

		if (Offset const *Beginning = CurrentActivity.GetBeginning())
			ActivityNode.put("Beginning", *Beginning, Translator);

		ScheduleNode.add_child("Activity", ActivityNode);
	}

	// Add to root
	Root.add_child("Schedule", ScheduleNode);

	try
	{
		boost::property_tree::xml_writer_settings<boost::property_tree::ptree::key_type> Settings('\t', 1);
		boost::property_tree::write_xml(FileName, Root, std::locale(), Settings);
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << "\n";
		return false;
	}

	return true;
}
