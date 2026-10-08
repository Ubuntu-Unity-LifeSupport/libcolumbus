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

#include "WordStore.hh"
#include "Word.hh"
#include <cassert>
#include <stdexcept>

using namespace Columbus;

void testStore() {
    WordStore s;
    Word w1("abc");
    Word w1Copy("abc");
    Word w2("def");
    WordID w1ID, w1CopyID, w2ID;
    WordID nonexisting = 42;
    bool gotException;

    w1ID = s.getID(w1);
    w1CopyID = s.getID(w1Copy);
    w2ID = s.getID(w2);

    assert(w1ID == w1CopyID);
    assert(w2ID != w1ID);

    assert(nonexisting != w1ID);
    assert(nonexisting != w2ID);

    assert(s.getWord(w1ID) == w1);
    assert(s.getWord(w2ID) == w2);
    try {
        s.getWord(nonexisting);
        gotException = false;
    } catch(std::out_of_range &e) {
        gotException = true;
    }
    assert(gotException);
}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testStore();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
