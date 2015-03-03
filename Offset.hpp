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

#include <iostream>

namespace Schedule
{
	class Offset;
	typedef Offset Duration;

	class Offset
	{
	public:
		Offset();
		Offset(long Hours, long Minutes, long Seconds, bool Negative = false);
		~Offset() { }

		// Returns normalized values of each denomination
		long GetHours() const;
		long GetMinutes() const;
		long GetSeconds() const;

		// Returns the entire Offset represented as the requested denomination.  Smaller denominations are truncated.
		long GetTotalMinutes() const;
		long GetTotalSeconds() const;

		Duration GetHourRemainder() const;
		Duration GetMinuteRemainder() const;

		Offset	operator+(Offset const &b) const;
		Offset &operator+=(Offset const &b);

		Offset	operator-(Offset const &b) const;
		Offset &operator-=(Offset const &b);

		Offset	operator*(float b) const;
		Offset &operator*=(float b);

		Offset	operator/(float b) const;
		Offset &operator/=(float b);

		bool operator<(Offset const &b) const;
		bool operator>(Offset const &b) const;
		bool operator<=(Offset const &b) const;
		bool operator>=(Offset const &b) const;
		bool operator==(Offset const &b) const;
		bool operator!=(Offset const &b) const;

		bool IsNegative() const;
		bool IsZero() const;

	private:
		long	Hours;
		short	Minutes;
		short	Seconds;

		short	Sign;

		void Initialize(long Hours, long Minutes, long Seconds, bool Negative);
	};


	inline std::ostream &operator<<(std::ostream &a, Offset const &b)
	{
		short const Sign = (b.IsNegative() ? -1 : 1);
		long const Hours = std::abs(b.GetHours());
		long const Minutes = std::abs(b.GetMinutes());
		long const Seconds = std::abs(b.GetSeconds());

		a << (Sign == -1 ? "-" : "");

		a << (Hours < 10 ? "0" : "") << Hours << ":" <<
			 (Minutes < 10 ? "0" : "") << Minutes << ":" <<
			 (Seconds < 10 ? "0" : "") << Seconds;

		return a;
	}
}
