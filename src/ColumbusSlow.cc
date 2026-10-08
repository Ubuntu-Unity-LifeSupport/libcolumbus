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

#include "ColumbusSlow.hh"
#include "ColumbusCore.hh"
#include <cstdlib>
#include <sys/stat.h>

COL_NAMESPACE_START
using namespace std;

static bool fileExists(const char *fname) {
    struct stat foo;
    return stat(fname, &foo) == 0;
}

string findDataFile(const string &baseName) {
    string empty, s;
    const char *varname = "COLUMBUS_DATADIR";
    const char *envvarDir;

    envvarDir = getenv(varname);
    if(envvarDir) {
        s = envvarDir;
        if(s[s.length()-1] != '/')
            s += "/";
        s += baseName;
    } else {
        s = COLUMBUS_DATADIR;
        s += baseName;
    }
    if(fileExists(s.c_str()))
        return s;

    return empty;
}

COL_NAMESPACE_END
