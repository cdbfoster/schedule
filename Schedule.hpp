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

#ifndef SCHEDULE_SCHEDULE
#define SCHEDULE_SCHEDULE

#include "Activity.hpp"
#include "Offset.hpp"

namespace Schedule
{
	class Schedule
	{
	public:
		Schedule(Duration const &Length = Duration(6, 0, 0));
		Schedule(Schedule &&Other);
		Schedule(Schedule const &) = delete;
		~Schedule();

		Duration	GetLength() const;
		void		SetLength(Duration const &Length);

		// STL-style list handling

		typedef ActivityList::value_type				value_type;
		typedef ActivityList::reference					reference;
		typedef ActivityList::const_reference			const_reference;
		typedef ActivityList::size_type					size_type;
		typedef ActivityList::iterator					iterator;
		typedef ActivityList::const_iterator			const_iterator;
		typedef ActivityList::reverse_iterator			reverse_iterator;
		typedef ActivityList::const_reverse_iterator	const_reverse_iterator;

		iterator		begin();
		const_iterator	begin() const;
		iterator		end();
		const_iterator	end() const;

		reverse_iterator		rbegin();
		const_reverse_iterator	rbegin() const;
		reverse_iterator		rend();
		const_reverse_iterator	rend() const;

		reference		front();
		const_reference	front() const;
		reference		back();
		const_reference	back() const;

		size_type	size() const;
		bool		empty() const;

		// push_back and insert take ownership of the added/inserted pointers
		void		push_back(value_type const &val);
		iterator	insert(iterator position, value_type const &val);
		iterator	erase(iterator position);
		iterator	erase(iterator first, iterator last);
		void		remove(value_type const &val);

		void BeginActivity(Activity &Activity, Offset const &Beginning);
		void ClearBeginning(Activity &Activity);

	private:
		friend class Activity;

		void Update();

	private:
		struct Implementation;

		Implementation *Data;
	};
}

#endif
