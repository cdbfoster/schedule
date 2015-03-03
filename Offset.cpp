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

#include "Offset.hpp"

using namespace Schedule;

Offset::Offset()
{
	this->Hours = 0;
	this->Minutes = 0;
	this->Seconds = 0;

	this->Sign = 1;
}


Offset::Offset(long Hours, long Minutes, long Seconds, bool Negative)
{
	this->Sign = 1;
	this->Initialize(Hours, Minutes, Seconds, Negative);
}


void Offset::Initialize(long Hours, long Minutes, long Seconds, bool Negative)
{
	// View everything in a "positive" context
	if (Negative)
	{
		Seconds *= -1;
		Minutes *= -1;
		Hours *= -1;
	}

	// Add any overflows to the next denomination
	Minutes += Seconds / 60;
	Hours += Minutes / 60;

	// Wrap to the maximum value in each denomination
	Seconds %= 60;
	Minutes %= 60;

	// Carry from the next denomination to fill any negatives
	if (Seconds < 0) { Minutes -= 1; Seconds += 60; }
	if (Minutes < 0) { Hours -= 1; Minutes += 60; }
	if (Hours < 0)
	{
		this->Sign = -1;
		this->Initialize(Hours, Minutes, Seconds, true);
		return;
	}

	// Now all values are positive and Sign contains the appropriate multiplier
	this->Hours = Hours;
	this->Minutes = (short)Minutes;
	this->Seconds = (short)Seconds;
}


long Offset::GetHours() const			{ return Sign * Hours; }
long Offset::GetMinutes() const		{ return Sign * Minutes; }
long Offset::GetSeconds() const		{ return Sign * Seconds; }


long Offset::GetTotalMinutes() const		{ return (60 * this->GetHours()) + this->GetMinutes(); }
long Offset::GetTotalSeconds() const		{ return (60 * this->GetTotalMinutes()) + this->GetSeconds(); }


Duration Offset::GetHourRemainder() const	{ return Duration(0, Minutes, Seconds, (Sign > 0 ? false : true)); }
Duration Offset::GetMinuteRemainder() const	{ return Duration(0, 0, Seconds, (Sign > 0 ? false : true)); }


Offset Offset::operator+(Offset const &b) const
{
	return Offset(this->GetHours() + b.GetHours(),
				  this->GetMinutes() + b.GetMinutes(),
				  this->GetSeconds() + b.GetSeconds());
}


Offset &Offset::operator+=(Offset const &b)
{
	*this = *this + b;
	return *this;
}


Offset Offset::operator-(Offset const &b) const
{
	return Offset(this->GetHours() - b.GetHours(),
				  this->GetMinutes() - b.GetMinutes(),
				  this->GetSeconds() - b.GetSeconds());
}


Offset &Offset::operator-=(Offset const &b)
{
	*this = *this - b;
	return *this;
}


Offset Offset::operator*(float b) const
{
	float const Hours			= b * this->GetHours();
	float const Minutes			= b * this->GetMinutes() + 60 * (Hours - (long)Hours);
	float const Seconds			= b * this->GetSeconds() + 60 * (Minutes - (long)Minutes);

	return Offset(Hours, Minutes, Seconds);
}


Offset &Offset::operator*=(float b)
{
	*this = *this * b;
	return *this;
}


Offset Offset::operator/(float b) const
{
	return *this * (1.0 / b);
}


Offset &Offset::operator/=(float b)
{
	*this = *this / b;
	return *this;
}


bool Offset::operator<(Offset const &b) const
{
	if (this->GetHours() == b.GetHours())
	{
		if (this->GetMinutes() == b.GetMinutes())
		{
			return this->GetSeconds() < b.GetSeconds();
		}
		else
			return this->GetMinutes() < b.GetMinutes();
	}
	else
		return this->GetHours() < b.GetHours();
}


bool Offset::operator>(Offset const &b) const
{
	if (this->GetHours() == b.GetHours())
	{
		if (this->GetMinutes() == b.GetMinutes())
		{
			return this->GetSeconds() > b.GetSeconds();
		}
		else
			return this->GetMinutes() > b.GetMinutes();
	}
	else
		return this->GetHours() > b.GetHours();
}


bool Offset::operator<=(Offset const &b) const
{
	return !(*this > b);
}


bool Offset::operator>=(Offset const &b) const
{
	return !(*this < b);
}


bool Offset::operator==(Offset const &b) const
{
	if (this->GetHours() != b.GetHours())
		return false;
	if (this->GetMinutes() != b.GetMinutes())
		return false;
	return this->GetSeconds() == b.GetSeconds();
}


bool Offset::operator!=(Offset const &b) const
{
	return !(*this == b);
}


bool Offset::IsNegative() const
{
	return (Sign < 0 ? true : false);
}


bool Offset::IsZero() const
{
	if (this->GetHours() != 0)
		return false;
	if (this->GetMinutes() != 0)
		return false;
	return this->GetSeconds() == 0;
}
