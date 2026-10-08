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

#ifndef ERRORMATRIX_HH_
#define ERRORMATRIX_HH_

#include"ColumbusCore.hh"

/*
 * A helper class for LevenshteinIndex to keep track of the
 * error values.
 *
 * This class only works because LevenshteinIndex does depth
 * first search. Breadth first search will break it completely.
 *
 * So don't use that then.
 */

COL_NAMESPACE_START

class ErrorMatrix final {
    size_t rows, columns;
    int **m;

public:
    ErrorMatrix(const size_t rows_, const size_t columns_, const int insertError, const int deletionError);
    ~ErrorMatrix();
    ErrorMatrix(const ErrorMatrix &em) = delete;
    const ErrorMatrix & operator=(const ErrorMatrix &other) = delete;


    void set(const size_t rowNum, const size_t colNum, const int error);
    // No bounds checking because this is in the hot path.
    inline int get(const size_t rowNum, const size_t colNum) const { return m[rowNum][colNum]; }
    int totalError(const size_t rowNum) const;
    int minError(const size_t rowNum) const;

};

COL_NAMESPACE_END

#endif /* ERRORMATRIX_HH_ */
