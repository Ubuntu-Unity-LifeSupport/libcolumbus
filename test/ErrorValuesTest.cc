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

#include "ErrorValues.hh"
#include "Word.hh"
#include <cassert>

using namespace Columbus;

void testError() {
    int smallError = 1;
    int defaultError = ErrorValues::getDefaultError();
    Letter l1 = 16;
    Letter l2 = 17;
    ErrorValues ev;
    ErrorValues ev2;

    assert(ev.getSubstituteError(l1, l2) == defaultError);
    ev.setError(l1, l2, smallError);
    assert(ev.getSubstituteError(l1, l2) == smallError);
    assert(ev.getSubstituteError(l2, l1) == smallError);

    assert(ev2.getSubstituteError(l2, l1) == defaultError);
    ev2.setError(l2, l1, smallError);
    assert(ev2.getSubstituteError(l1, l2) == smallError);
    assert(ev2.getSubstituteError(l2, l1) == smallError);

    ev.clearErrors();
    assert(ev.getSubstituteError(l1, l2) == defaultError);
}

void testGroupError() {
    ErrorValues ev;
    Letter e = 'e'; // These must be in lower case.
    Letter eacute = 0xe9;
    Letter ebreve = 0x115;
    Letter a = 'a';
    Letter aacute = 0xe1;
    Letter abreve = 0x103;
    const int defaultError = ErrorValues::getDefaultError();
    const int defaultGroupError = ErrorValues::getDefaultGroupError();
    assert(defaultError != defaultGroupError);

    assert(ev.getSubstituteError(e, eacute) == defaultError);
    assert(ev.getSubstituteError(a, aacute) == defaultError);
    assert(ev.getSubstituteError(e, aacute) == defaultError);

    ev.addAccents(latinAccentGroup);
    assert(ev.isInGroup(e));
    assert(ev.isInGroup(eacute));
    assert(ev.isInGroup(ebreve));
    assert(ev.isInGroup(a));
    assert(ev.isInGroup(aacute));
    assert(ev.isInGroup(abreve));

    assert(ev.getSubstituteError(e, eacute) == defaultGroupError);
    assert(ev.getSubstituteError(eacute, e) == defaultGroupError);
    assert(ev.getSubstituteError(eacute, ebreve) == defaultGroupError);
    assert(ev.getSubstituteError(e, ebreve) == defaultGroupError);

    assert(ev.getSubstituteError(a, e) == defaultError);
    assert(ev.getSubstituteError(a, aacute) == defaultGroupError);
    assert(ev.getSubstituteError(abreve, aacute) == defaultGroupError);

    assert(ev.getSubstituteError(eacute, aacute) == defaultError);

    ev.clearErrors();
    assert(ev.getSubstituteError(e, eacute) == defaultError);
}

void testKeyboardErrors() {
    ErrorValues ev;
    const int defaultError = ErrorValues::getDefaultError();
    const int typoError = ErrorValues::getDefaultTypoError();

    assert(ev.getSubstituteError('q', 'a') == defaultError);

    ev.addKeyboardErrors();
    assert(ev.getSubstituteError('q', 'a') == typoError);
    assert(ev.getSubstituteError('w', 'a') == typoError);
}

void testNumberpadErrors() {
    ErrorValues ev;
    ev.addNumberpadErrors();

    assert(ev.getSubstituteError('2', 'a') == 0);
    assert(ev.getSubstituteError('5', 'm') < ErrorValues::getDefaultError());
    assert(ev.getSubstituteError('j', '6') < ErrorValues::getDefaultError());
}

void testBigError() {
    ErrorValues ev;
    Letter l1 = 1000;  // Big values, so they are guaranteed to be outside of the LUT.
    Letter l2 = 10000;
    int smallError = 1;

    assert(smallError < ErrorValues::getDefaultError());
    assert(ev.getSubstituteError(l1, l2) == ErrorValues::getDefaultError());
    assert(ev.getSubstituteError(l2, l1) == ErrorValues::getDefaultError());
    assert(ev.getSubstituteError(l2, l2) == 0);

    ev.setError(l1, l2, smallError);
    assert(ev.getSubstituteError(l1, l2) == smallError);
    assert(ev.getSubstituteError(l2, l1) == smallError);
    assert(ev.getSubstituteError(l2, l2) == 0);

}

int main(int /*argc*/, char **/*argv*/) {
    try {
        testError();
        testGroupError();
        testKeyboardErrors();
        testNumberpadErrors();
        testBigError();
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}

