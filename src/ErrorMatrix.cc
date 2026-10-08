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

#include <stdexcept>
#include "ErrorMatrix.hh"

using namespace std;

COL_NAMESPACE_START

ErrorMatrix::ErrorMatrix(const size_t rows_, const size_t columns_, const int insertError, const int deletionError) :
    rows(rows_),
    columns(columns_) {
    m = new int*[rows+1];
    for(size_t i=0; i<=rows; i++) {
        m[i] = new int[columns+1];
    }
    for(size_t i=0; i<=columns; i++) {
        m[0][i] = i*insertError;
    }
    for(size_t i=1; i<=rows; i++)
        m[i][0] = m[i-1][0] + deletionError;
}

ErrorMatrix::~ErrorMatrix() {
    for(size_t i=0; i<=rows; i++) {
        delete []m[i];
    }
    delete []m;

}

void ErrorMatrix::set(const size_t rowNum, const size_t colNum, const int error) {
    if(rowNum > rows)
        throw out_of_range("Illegal row number.");
    if(colNum > columns)
        throw out_of_range("Illegal column number.");
    m[rowNum][colNum] = error;
}

int ErrorMatrix::totalError(const size_t rowNum) const {
    return m[rowNum][columns-1];
}

int ErrorMatrix::minError(const size_t rowNum) const {
    int result = m[rowNum][0];
    for(size_t i=1; i<columns; i++)
        if(m[rowNum][i] < result)
            result = m[rowNum][i];
    return result;
}

COL_NAMESPACE_END
