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

/*
 * This file contains helper functions that for some reason or
 * another must include libc++ stuff. Whenever this file is included
 * compilation times slow down.
 *
 * If a function can be removed from here, it should.
 */

#ifndef COLUMBUSSLOW_HH
#define COLUMBUSSLOW_HH

#include "ColumbusCore.hh"

#include<string>

COL_NAMESPACE_START

std::string findDataFile(const std::string &baseName);

COL_NAMESPACE_END

#endif
