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

#include <algorithm>
#include <set>

#include "Schedule.hpp"

using namespace Schedule;

struct Schedule::Schedule::Implementation
{
	Implementation(Duration const &Length)
	{
		Activity *EndActivity = new Activity;
		EndActivity->SetName("End");
		EndActivity->SetDesiredStartTime(Length);
		EndActivity->SetStartMode(Activity::StartMode::FIXED_RELATIVE);
		EndActivity->SetDesiredLength(Duration());
		EndActivity->SetLengthMode(Activity::LengthMode::FIXED);

		this->Activities.push_back(EndActivity);
		this->EndActivity = this->Activities.begin();
	}

	~Implementation()
	{
		for (ActivityList::const_iterator Activity = this->Activities.begin(); Activity != this->Activities.end(); ++Activity)
			delete *Activity;
	}

	ActivityList			Activities;
	ActivityList::iterator	EndActivity;

	void SetLength(Duration const &Length);

	void AddActivity(Activity *Add);
	void InsertActivity(Activity *Insert, Activity &Before);
	bool RemoveActivity(Activity &Remove);
};


Schedule::Schedule::Schedule(Duration const &Length) :
	Data(new Implementation(Length))
{

}


Schedule::Schedule::Schedule(Schedule &&Other) :
	Data(Other.Data)
{
	Other.Data = nullptr;

	for (auto Activity : this->Data->Activities)
		Activity->Owner = this;
}


Schedule::Schedule::~Schedule()
{
	if (this->Data != nullptr)
		delete this->Data;
}


Duration Schedule::Schedule::GetLength() const { return (*this->Data->EndActivity)->GetDesiredStartTime(); }


void Schedule::Schedule::SetLength(Duration const &Length)
{
	this->Data->SetLength(Length);
	this->Update();
}


Schedule::Schedule::iterator Schedule::Schedule::begin()
{
	return this->Data->Activities.begin();
}


Schedule::Schedule::const_iterator Schedule::Schedule::begin() const
{
	return this->Data->Activities.begin();
}


Schedule::Schedule::iterator Schedule::Schedule::end()
{
	iterator Result = this->Data->Activities.end();
	std::advance(Result, -1);
	return Result;
}


Schedule::Schedule::const_iterator Schedule::Schedule::end() const
{
	const_iterator Result = this->Data->Activities.end();
	std::advance(Result, -1);
	return Result;
}


Schedule::Schedule::reverse_iterator Schedule::Schedule::rbegin()
{
	reverse_iterator Result = this->Data->Activities.rbegin();
	std::advance(Result, 1);
	return Result;
}


Schedule::Schedule::const_reverse_iterator Schedule::Schedule::rbegin() const
{
	const_reverse_iterator Result = this->Data->Activities.rbegin();
	std::advance(Result, 1);
	return Result;
}


Schedule::Schedule::reverse_iterator Schedule::Schedule::rend()
{
	return this->Data->Activities.rend();
}


Schedule::Schedule::const_reverse_iterator Schedule::Schedule::rend() const
{
	return this->Data->Activities.rend();
}


Schedule::Schedule::reference Schedule::Schedule::front()
{
	return this->Data->Activities.front();
}


Schedule::Schedule::const_reference Schedule::Schedule::front() const
{
	return this->Data->Activities.front();
}


Schedule::Schedule::reference Schedule::Schedule::back()
{
	iterator Back = this->end();
	std::advance(Back, -1);
	return *Back;
}


Schedule::Schedule::const_reference Schedule::Schedule::back() const
{
	const_iterator Back = this->end();
	std::advance(Back, -1);
	return *Back;
}


Schedule::Schedule::size_type Schedule::Schedule::size() const
{
	return this->Data->Activities.size() - 1;
}


bool Schedule::Schedule::empty() const
{
	return this->size() == 0;
}


void Schedule::Schedule::push_back(value_type const &val)
{
	if (val->Owner != NULL && val->Owner != this)
		val->Owner->remove(val);

	val->Owner = this;

	if (std::find(this->Data->Activities.begin(), this->Data->Activities.end(), val) != this->Data->Activities.end())
		std::cerr << "push_back: Activity already exists in schedule." << std::endl;

	this->Data->Activities.insert(this->end(), val);
	this->Update();
}


Schedule::Schedule::iterator Schedule::Schedule::insert(iterator position, value_type const &val)
{
	if (val->Owner != NULL && val->Owner != this)
		val->Owner->remove(val);

	val->Owner = this;

	if (std::find(this->Data->Activities.begin(), this->Data->Activities.end(), val) != this->Data->Activities.end())
		std::cerr << "insert: Activity already exists in schedule." << std::endl;

	iterator Result = this->Data->Activities.insert(position, val);

	this->Update();

	return Result;
}


Schedule::Schedule::iterator Schedule::Schedule::erase(iterator position)
{
	if (position == this->end())
		return this->end();

	(*position)->Owner = NULL;
	iterator Result = this->Data->Activities.erase(position);

	this->Update();

	return Result;
}


Schedule::Schedule::iterator Schedule::Schedule::erase(iterator first, iterator last)
{
	iterator Current = first;

	while (Current != last)
		Current = this->erase(Current);

	return Current;
}


void Schedule::Schedule::remove(value_type const &val)
{
	this->Data->Activities.remove(val);

	this->Update();
}


void Schedule::Schedule::BeginActivity(Activity &Activity, Offset const &Beginning)
{
	if (Activity.Owner != this)
		std::cerr << "Attempting to begin an activity that doesn't belong to this schedule." << std::endl;

	Activity.SetBeginning(Beginning);
	this->Update();
}


void Schedule::Schedule::ClearBeginning(Activity &Activity)
{
	if (Activity.Owner != this)
		std::cerr << "Attempting to begin an activity that doesn't belong to this schedule." << std::endl;

	Activity.ClearBeginning();
	this->Update();
}


void Schedule::Schedule::Update()
{
	// There must be at least one other activity besides the End activity
	if (this->Data->Activities.size() == 1)
		return;

	// Convenience
	ActivityList const &Activities = this->Data->Activities;

	// The first activity must be fixed-absolute
	Activities.front()->ActivityStartMode = Activity::StartMode::FIXED_ABSOLUTE;


	// Constants
	Duration const ScheduleLength = (*this->Data->EndActivity)->GetDesiredStartTime();

	Offset const StartTime = (Activities.front()->GetBeginning() != nullptr ? *Activities.front()->GetBeginning() :
																			   Activities.front()->GetDesiredStartTime());
	Offset const EndTime = StartTime + ScheduleLength;


	// Set fixed attributes to actual (start times, beginnings, lengths)
	{
		// Activities that contain some fixed attribute
		std::vector<Activity *> FixedActivities;

		for (auto CurrentActivity : Activities)
		{
			if (CurrentActivity->GetStartMode() != Activity::StartMode::FREE || CurrentActivity->GetBeginning() != nullptr ||
				CurrentActivity->GetLengthMode() != Activity::LengthMode::FREE)
			{
				FixedActivities.push_back(CurrentActivity);
			}
		}

		std::vector<Activity *>::const_iterator PreviousBeginning = FixedActivities.end();
		std::vector<Activity *>::const_iterator FixedActivityIterator = FixedActivities.begin();
		bool FlexibleLength = true;
		Offset CurrentTime = StartTime;

		// Set fixed to actual.  If there's a dispute, beginnings take priority, followed by lengths, followed by start times.
		for (auto CurrentActivity : Activities)
		{
			// This activity has a beginning
			if (Offset const * const Beginning = CurrentActivity->GetBeginning())
			{
				// If this activity begins before a previous one allows,
				if (*Beginning < CurrentTime && PreviousBeginning != FixedActivities.end())
				{
					Offset AdjustTime = (*PreviousBeginning)->GetActualStartTime();

					// If the previous beginning is not the issue, chop off the offending time from the activities between
					// this beginning and the previous
					if (*Beginning >= AdjustTime)
					{
						for (std::vector<Activity *>::const_iterator AdjustActivityIterator = PreviousBeginning;
																	 AdjustActivityIterator != FixedActivityIterator;
																	 ++AdjustActivityIterator)
						{
							Activity * const AdjustActivity = *AdjustActivityIterator;

							if (AdjustActivity->GetStartMode() != Activity::StartMode::FREE)
							{
								AdjustTime = AdjustActivity->GetActualStartTime();

								if (*Beginning < AdjustTime)
								{
									AdjustTime = *Beginning;
									AdjustActivity->SetActualStartTime(AdjustTime);
								}
							}

							if (AdjustActivity->GetLengthMode() != Activity::LengthMode::FREE)
							{
								Offset const OldAdjustTime = AdjustTime;
								AdjustTime += AdjustActivity->GetActualLength();

								if (*Beginning < AdjustTime)
								{
									AdjustTime = *Beginning;
									AdjustActivity->SetActualLength(AdjustTime - OldAdjustTime);
								}
							}
						}

						if (*Beginning > EndTime)
							CurrentActivity->SetActualStartTime(EndTime);
						else
							CurrentActivity->SetActualStartTime(*Beginning);

						CurrentTime = CurrentActivity->GetActualStartTime();
					}
					// Otherwise, this beginning wants to be before the previous beginning.  The previous beginning wins.
					else
					{
						CurrentActivity->SetActualStartTime(AdjustTime);
						CurrentTime = AdjustTime;
					}
				}
				// No conflict.  Set the beginning where desired.
				else
				{
					if (*Beginning > EndTime)
						CurrentActivity->SetActualStartTime(EndTime);
					else
						CurrentActivity->SetActualStartTime(*Beginning);

					CurrentTime = CurrentActivity->GetActualStartTime();
				}

				PreviousBeginning = FixedActivityIterator;
				FlexibleLength = false;
			}
			// This activity has a fixed start, but is not yet begun
			else if (CurrentActivity->GetStartMode() != Activity::StartMode::FREE)
			{
				Offset const DesiredStartTime = (CurrentActivity->GetStartMode() == Activity::StartMode::FIXED_ABSOLUTE ?
																					CurrentActivity->GetDesiredStartTime() :
																					CurrentActivity->GetDesiredStartTime() + StartTime);

				// Yield to previous fixed starts/lengths
				if (DesiredStartTime < CurrentTime || !FlexibleLength)
				{
					CurrentActivity->SetActualStartTime(CurrentTime);
				}
				else
				{
					if (DesiredStartTime > EndTime)
						CurrentActivity->SetActualStartTime(EndTime);
					else
						CurrentActivity->SetActualStartTime(DesiredStartTime);

					CurrentTime = CurrentActivity->GetActualStartTime();
				}

				FlexibleLength = false;
			}

			Duration const RemainingTime = EndTime - CurrentTime;

			// Set fixed lengths to actual
			if (CurrentActivity->GetLengthMode() == Activity::LengthMode::FIXED)
			{
				Duration const DesiredLength = CurrentActivity->GetDesiredLength();

				if (DesiredLength > RemainingTime)
				{
					CurrentActivity->SetActualLength(RemainingTime);
					CurrentTime = EndTime;
				}
				else
				{
					CurrentActivity->SetActualLength(DesiredLength);
					CurrentTime += DesiredLength;
				}
			}
			else
				FlexibleLength = true;

			// Keep the position of the current activity in the fixed list
			if (CurrentActivity->GetStartMode() != Activity::StartMode::FREE || CurrentActivity->GetBeginning() != nullptr ||
				CurrentActivity->GetLengthMode() != Activity::LengthMode::FREE)
			{
				++FixedActivityIterator;
			}
		}
	}


	// Stretch the non-fixed activities to fill the spaces between the fixed activities
	{
		Duration ExpandedLength;	// The length of the flexible activities before scaling
		Duration FixedLength;		// The amount of the space between fixed activities that cannot stretch

		Offset CurrentTime = StartTime;

		ActivityList::const_iterator LowerBound = Activities.end();

		for (ActivityList::const_iterator CurrentActivityIterator = Activities.begin();
										  CurrentActivityIterator != Activities.end();
										  ++CurrentActivityIterator)
		{
			Activity * const CurrentActivity = *CurrentActivityIterator;

			// If there's a fixed start or a beginning, we've found a boundary of some fixed space
			if (CurrentActivity->GetStartMode() != Activity::StartMode::FREE || CurrentActivity->GetBeginning() != nullptr)
			{
				// If this isn't the very first boundary,
				if (LowerBound != Activities.end())
				{
					ActivityList::const_iterator const UpperBound = CurrentActivityIterator;

					// Calculate the scale to be applied to the activities inside the boundaries
					Duration const FlexibleLength = (*UpperBound)->GetActualStartTime() - (*LowerBound)->GetActualStartTime() - FixedLength;

					float const FlexibleScale = (!ExpandedLength.IsZero() ? (float)FlexibleLength.GetTotalSeconds() / (float)ExpandedLength.GetTotalSeconds() :
																			0.0f);

					// Apply the scale to the free-length activities
					for (auto CurrentActivity : std::vector<Activity *>(LowerBound, UpperBound))
					{
						if (CurrentActivity->GetStartMode() == Activity::StartMode::FREE && CurrentActivity->GetBeginning() == nullptr)
							CurrentActivity->SetActualStartTime(CurrentTime);

						if (CurrentActivity->GetLengthMode() == Activity::LengthMode::FREE)
						{
							Duration ActualLength = CurrentActivity->GetDesiredLength() * FlexibleScale;

							if (ActualLength.IsNegative())
								ActualLength = Duration();

							CurrentActivity->SetActualLength(ActualLength);
						}

						CurrentTime += CurrentActivity->GetActualLength();
					}

					// Reset for the next pair of boundaries
					ExpandedLength = Duration();
					FixedLength = Duration();
					LowerBound = UpperBound;
				}
				else
					LowerBound = CurrentActivityIterator;
			}

			if (CurrentActivity->GetLengthMode() == Activity::LengthMode::FREE)
				ExpandedLength += CurrentActivity->GetDesiredLength();
			else
				FixedLength += CurrentActivity->GetActualLength();
		}
	}
}


void Schedule::Schedule::Implementation::SetLength(Duration const &Length)
{
	(*this->EndActivity)->SetDesiredStartTime(Length);
}


void Schedule::Schedule::Implementation::AddActivity(Activity *Add)
{
	ActivityList::iterator FindActivity = std::find(this->Activities.begin(), this->Activities.end(), Add);

	if (FindActivity != this->Activities.end())
		this->Activities.erase(FindActivity);

	this->Activities.insert(this->EndActivity, Add);
}


void Schedule::Schedule::Implementation::InsertActivity(Activity *Insert, Activity &Before)
{
	ActivityList::iterator FindActivity = std::find(this->Activities.begin(), this->Activities.end(), Insert);

	if (FindActivity != this->Activities.end())
		this->Activities.erase(FindActivity);

	ActivityList::iterator BeforeActivity = std::find(this->Activities.begin(), this->Activities.end(), &Before);

	if (BeforeActivity == this->Activities.end())
		this->Activities.insert(this->EndActivity, Insert);
	else
		this->Activities.insert(BeforeActivity, Insert);
}


bool Schedule::Schedule::Implementation::RemoveActivity(Activity &Remove)
{
	for (ActivityList::iterator RemoveActivity = this->Activities.begin(); RemoveActivity != this->Activities.end(); ++RemoveActivity)
	{
		if (*RemoveActivity == &Remove)
		{
			this->Activities.erase(RemoveActivity);
			return true;
		}
	}

	return false;
}
