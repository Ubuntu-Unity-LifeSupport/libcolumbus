/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include"SearchParameters.hh"
#include"Word.hh"
#include"LevenshteinIndex.hh"
#include"ResultFilter.hh"
#include<set>

COL_NAMESPACE_START

using namespace std;
struct SearchParametersPrivate {
    bool dynamic;
    ResultFilter filter;
    set<Word> nosearchFields;
};

SearchParameters::SearchParameters() {
    p = new SearchParametersPrivate();
    p->dynamic = true;
}

SearchParameters::~SearchParameters() {
    delete p;
}

bool SearchParameters::isDynamic() const {
    return p->dynamic;
}
void SearchParameters::setDynamic(bool dyn) {
    p->dynamic = dyn;
}

/*
 * Long words should allow for more error than short ones.
 * This is a simple function which is meant to be strict
 * so there won't be too many matches.
 */

int SearchParameters::getDynamicError(const Word &w) const {
    size_t len = w.length();
    if(len < 2)
        return LevenshteinIndex::getDefaultError();
    else
        return 2*LevenshteinIndex::getDefaultError();
}

ResultFilter& SearchParameters::getResultFilter() {
    return p->filter;
}

const ResultFilter& SearchParameters::getResultFilter() const {
    return p->filter;
}

void SearchParameters::addNonsearchingField(const Word &w) {
    p->nosearchFields.insert(w);
}

bool SearchParameters::isNonsearchingField(const Word &w) const {
    return p->nosearchFields.find(w) != p->nosearchFields.end();
}

int SearchParameters::looseningIterations() const {
    return 1;
}

COL_NAMESPACE_END

