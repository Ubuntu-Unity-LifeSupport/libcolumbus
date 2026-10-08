#!/usr/bin/python3 -tt
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Canonical, Ltd.

# Authors:
#    Jussi Pakkanen <jussi.pakkanen@canonical.com>

# This library is free software; you can redistribute it and/or modify it under
# the terms of version 3 of the GNU Lesser General Public License as published
# by the Free Software Foundation.

# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sysconfig
print(sysconfig.get_config_var("SOABI"))

