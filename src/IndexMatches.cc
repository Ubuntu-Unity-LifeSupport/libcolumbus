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

#include "IndexMatches.hh"
#include "Word.hh"

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cassert>

COL_NAMESPACE_START
using namespace std;

struct MatchData {
    Word queryWord;
    WordID matchedWord;
    int error;

    bool operator<(const MatchData &other) const {
        return error < other.error;
    }
};

struct IndexMatchesPrivate {
    vector<MatchData> matches;
};

IndexMatches::IndexMatches() {
    p = new IndexMatchesPrivate();
}

IndexMatches::~IndexMatches() {
    delete p;
}

void IndexMatches::addMatch(const Word &/*queryWord*/, const WordID matchedWord, int error) {
    MatchData m;
    m.matchedWord = matchedWord;
    m.error = error;
    p->matches.push_back(m);
}

size_t IndexMatches::size() const {
    return p->matches.size();
}

const WordID& IndexMatches::getMatch(size_t num) const {
    if(num >= p->matches.size()) {
        std::string msg("Attempt to access match ");
        msg += num;
        msg += " out of bounds (array size ";
        msg += p->matches.size();
        msg += ").";
        throw out_of_range(msg);
    }
    return p->matches[num].matchedWord;
}

const Word& IndexMatches::getQuery(size_t num) const {
    if(num >= p->matches.size()) {
        std::string msg("Attempt to access query term ");
        msg += num;
        msg += " out of bounds (array size ";
        msg += p->matches.size();
        msg += ").";
        throw out_of_range(msg);
    }
    return p->matches[num].queryWord;
}

int IndexMatches::getMatchError(size_t num) const {
    if(num >= p->matches.size()) {
        std::string msg("Attempt to access match error ");
        msg += num;
        msg += " out of bounds (array size ";
        msg += p->matches.size();
        msg += ").";
        throw out_of_range(msg);
    }
    return p->matches[num].error;
}

void IndexMatches::clear() {
    p->matches.clear();
}

void IndexMatches::sort() {
    std::sort(p->matches.begin(), p->matches.end());
}

COL_NAMESPACE_END
