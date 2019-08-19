/*! \file general_io/errors.h
 *  \defgroup general_io_group General I/O card
 *  \brief Main file of the General I/O card.
 *  \author Mikael Larsmark, SM2WMV
 *  \date 2010-05-18
 *  \code #include "general_io/errors.h" \endcode
 */
//    Copyright (C) 2016  Mikael Larsmark, SM2WMV
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _ERRORS_H_
#define _ERRORS_H_

//The rotator has not rotated the at least the correct amount of degrees
//in a certain time
#define ERROR_ROTATOR_STUCK		        0
#define ERROR_ROTATOR_END_LIMIT_CW    1
#define ERROR_ROTATOR_END_LIMIT_CCW   2


#endif