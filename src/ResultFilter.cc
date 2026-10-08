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


#include "ResultFilter.hh"
#include "Word.hh"
#include <vector>
#include <stdexcept>

COL_NAMESPACE_START
using namespace std;

struct ResultFilterPrivate {
    vector<vector<pair<Word, Word>>> termList;
};

ResultFilter::ResultFilter() {
    p = new ResultFilterPrivate();
    addNewTerm();
}

ResultFilter::~ResultFilter() {
    delete p;
}

void ResultFilter::addNewTerm() {
    vector<pair<Word, Word> > dummy;
    p->termList.push_back(dummy);
}

void ResultFilter::addNewSubTerm(const Word &field, const Word &word) {
    p->termList.back().push_back(make_pair(field, word));
}

const Word& ResultFilter::getField(const size_t term, const size_t subTerm) const {
    if(term >= numTerms())
        throw out_of_range("Term access out of bounds in ResultFilter::getField.");
    if(subTerm >= p->termList[term].size())
        throw out_of_range("Subterm access out of bounds in ResultFilter::getField.");
    return p->termList[term][subTerm].first;
}

const Word& ResultFilter::getWord(const size_t term, const size_t subTerm) const {
    if(term >= numTerms())
        throw out_of_range("Term access out of bounds in ResultFilter::getField.");
    if(subTerm >= p->termList[term].size())
        throw out_of_range("Subterm access out of bounds in ResultFilter::getField.");
    return p->termList[term][subTerm].second;
}

size_t ResultFilter::numTerms() const {
    return p->termList.size();
}

size_t ResultFilter::numSubTerms(const size_t term) const {
    if(term >= numTerms())
        throw out_of_range("Access out of bounds in ResultFilter::numSubTerms.");
    return p->termList[term].size();
}

COL_NAMESPACE_END

