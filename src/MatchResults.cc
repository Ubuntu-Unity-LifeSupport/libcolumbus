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

#include "MatchResults.hh"
#include "Word.hh"
#include <vector>
#include <algorithm>
#include <stdexcept>

COL_NAMESPACE_START
using namespace std;

struct MatchResultsPrivate {
    vector<pair<double, DocumentID> > results;
    bool sorted;
};

MatchResults::MatchResults() {
    p = new MatchResultsPrivate();
    p->sorted = true;;
}

MatchResults::MatchResults(const MatchResults &other) {
    p = new MatchResultsPrivate();
    *p = *other.p;
}

MatchResults::MatchResults(MatchResults &&other) {
    p = other.p;
    other.p = nullptr;
}

MatchResults::~MatchResults() {
    delete p;
}

const MatchResults& MatchResults::operator=(MatchResults &&other) {
    if(this != &other) {
        delete p;
        p = other.p;
        other.p = nullptr;
    }
    return *this;
}

const MatchResults& MatchResults::operator=(const MatchResults &other) {
    if(this != &other) {
        *p = *other.p;
    }
    return *this;
}

void MatchResults::addResult(DocumentID id, double relevancy) {
    pair<double, DocumentID> n;
    n.first = relevancy;
    n.second = id;
    p->results.push_back(n);
    p->sorted = false;
}

void MatchResults::addResults(const MatchResults &r) {
    p->results.insert(p->results.end(), r.p->results.begin(), r.p->results.end());
    p->sorted = false;
}


size_t MatchResults::size() const {
    return p->results.size();
}

void MatchResults::copyResult(const MatchResults &other, const size_t i) {
    if(i >= other.p->results.size()) {
        throw out_of_range("Tried to copy an out-of-range result.");
    }
    p->results.push_back(other.p->results[i]);
    p->sorted = false;
}

void MatchResults::sortIfRequired() const {
    if(p->sorted)
        return;
    MatchResults *me = const_cast<MatchResults*>(this);
    stable_sort(me->p->results.rbegin(), me->p->results.rend(),
            [](const pair<double, DocumentID> &a, const pair<double, DocumentID> &b) -> bool{
        return a.first < b.first;
    });
    me->p->sorted = true;
}

DocumentID MatchResults::getDocumentID(size_t i) const {
    if(i>=p->results.size()) {
        throw out_of_range("Access out of bounds in MatchResults::getDocumentID.");
    }
    sortIfRequired();
    return p->results[i].second;
}

double MatchResults::getRelevancy(size_t i) const {
    if(i>=p->results.size()) {
        throw out_of_range("Access out of bounds in MatchResults::getDocumentID.");
    }
    sortIfRequired();
    return p->results[i].first;
}

COL_NAMESPACE_END
