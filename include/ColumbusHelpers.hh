/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COLUMBUSHELPERS_H_
#define COLUMBUSHELPERS_H_

#include "ColumbusCore.hh"

COL_NAMESPACE_START

class Word;
class WordList;

Letter* utf8ToInternal(const char *utf8Text, unsigned int &resultStringSize);
void internalToUtf8(const Letter *source, unsigned int characters, char *buf, unsigned int bufsize);
COL_PUBLIC COL_PUBLIC double hiresTimestamp();
COL_PUBLIC WordList splitToWords(const char *utf8Text);
COL_PUBLIC WordList split(const char *utf8Text, const Letter *splitChars, int numChars);
COL_PUBLIC bool isWhitespace(Letter l);

COL_NAMESPACE_END

#endif

