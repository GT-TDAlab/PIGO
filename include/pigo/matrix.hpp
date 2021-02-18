/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This contains the Matrix class, interfacing to a CSR
 */

#ifndef PIGO_MATRIX_HPP
#define PIGO_MATRIX_HPP

#include <string>
#include <memory>

namespace pigo {

    /** @brief Used to load matrices from disk
     *
     * This class provides a matrix-specific naming on top of CSRs.
     *
     * @tparam col_t the type to use for column positions
     * @tparam row_t the type to use for row positions
     * @tparam col_storage the storage type of the endpoints of the CSR.
     * @tparam row_storage the storage type of the offsets of the
     *         compressed row.
     */
    template<
        class col_t=uint32_t,
        class row_t=uint32_t,
        class col_storage=col_t*,
        class row_storage=row_t*
    >
    class BaseMatrix : public CSR<col_t, row_t,
            col_storage, row_storage> {
        public:
            using CSR<col_t, row_t, col_storage, row_storage>::CSR;
    };

    using Matrix = BaseMatrix<>;

}

#endif
