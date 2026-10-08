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
#include <cassert>

using namespace Columbus;

void testMatchResult() {
    MatchResults r;
    DocumentID w1 = 0;
    double r1 = 1;
    DocumentID w2 = 1;
    double r2 = 2;
    DocumentID w3 = 2;
    double r3 = 0.5;

    assert(r.size() == 0);
    r.addResult(w1, r1);
    assert(r.size() == 1);
    assert(r.getRelevancy(0) == r1);

    r.addResult(w2, r2);
    assert(r.size() == 2);
    assert(r.getRelevancy(0) == r2);

    r.addResult(w3, r3);
    assert(r.size() == 3);
    assert(r.getRelevancy(0) == r2);
}

MatchResults gimme() {
    MatchResults m;
    m.addResult(1, 1);
    m.addResult(2, 2);
    return m;
}

/*
 *  For great Valgrind justice.
 */
void testAssignments() {
    MatchResults m1, m2;
    m1.addResult(3, 4);
    m2 = m1;
    MatchResults m3(m1);
    MatchResults m4(m3);
    MatchResults m5(gimme());
    MatchResults m6 = gimme();
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testMatchResult();
        testAssignments();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
