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
 * Columbus is an error tolerant matcher system.
 *
 * Include this file to use the C++ API to Columbus.
 *
 * If you need a C api, include columbus.h instead.
 * It is not guaranteed to have all functionality, though.
 */

#ifndef COLUMBUS_HH_
#define COLUMBUS_HH_

#ifdef COLUMBUS_H_
#error "Mixing C and C++ public header includes. You can only use one or the other."
#endif

#include <Matcher.hh>
#include <MatchResults.hh>
#include <Corpus.hh>
#include <Word.hh>
#include <WordList.hh>
#include <Document.hh>
#include <ColumbusHelpers.hh>
#include <IndexWeights.hh>
#include <ErrorValues.hh>

#endif
