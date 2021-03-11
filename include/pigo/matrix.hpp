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

    /** @brief Used to load symmetric matrices from disk
     *
     * This class provides a matrix-specific naming on top of CSRs.
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest value read inside of the Matrix.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of non-zeros
     *         in the Matrix. It defaults to Label.
     * @tparam LabelStorage the storage type of the endpoints of the CSR.
     *         This can either be vector (std::vector<Label>),
     *         a pointer (Label*), or a shared_ptr
     *         (std::shared_ptr<Label>).
     * @tparam OrdinalStorage the storage type of the offsets of the CSR.
     *         This can either be vector (std::vector<Ordinal>),
     *         a pointer (Ordinal*), or a shared_ptr
     *         (std::shared_ptr<Ordinal>).
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type.
     * @tparam WeightStorage the storage type for the weights. This can be
     *         a raw pointer (Weight*), a std::vector
     *         (std::vector<Weight>), or a std::shared_ptr<Weight>.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class LabelStorage=Label*,
        class OrdinalStorage=Ordinal*,
        bool weighted=false,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class SymMatrix: public CSR<
                        Label,
                        Ordinal,
                        LabelStorage,
                        OrdinalStorage,
                        weighted,
                        Weight,
                        WeightStorage
                     > {
        public:
            using CSR<
                    Label,
                    Ordinal,
                    LabelStorage,
                    OrdinalStorage,
                    weighted,
                    Weight,
                    WeightStorage
                >::CSR;
    };

    /** @brief Used to hold regular (non-symmetric) matrices
     *
     * This contains both a CSR and a CSC
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest value read inside of the Matrix.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of non-zeros
     *         in the Matrix. It defaults to Label.
     * @tparam LabelStorage the storage type of the endpoints of the
     *         CSR/CSC.
     *         This can either be vector (std::vector<Label>),
     *         a pointer (Label*), or a shared_ptr
     *         (std::shared_ptr<Label>).
     * @tparam OrdinalStorage the storage type of the offsets of the
     *         CSR/CSC.
     *         This can either be vector (std::vector<Ordinal>),
     *         a pointer (Ordinal*), or a shared_ptr
     *         (std::shared_ptr<Ordinal>).
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type.
     * @tparam WeightStorage the storage type for the weights. This can be
     *         a raw pointer (Weight*), a std::vector
     *         (std::vector<Weight>), or a std::shared_ptr<Weight>.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class LabelStorage=Label*,
        class OrdinalStorage=Ordinal*,
        bool weighted=false,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class Matrix {
        private:
            /** Holds the CSR, allowing for row-based access */
            CSR<
                    Label,
                    Ordinal,
                    LabelStorage,
                    OrdinalStorage,
                    weighted,
                    Weight,
                    WeightStorage
                > csr_;

            /** Holds the CSC, allowing for col-based access */
            CSC<
                    Label,
                    Ordinal,
                    LabelStorage,
                    OrdinalStorage,
                    weighted,
                    Weight,
                    WeightStorage
                > csc_;

            /** @brief Build a Matrix from a COO */
            template <class COOLabel, class COOOrdinal, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl, bool COOme,
                     class COOW, class COOWS>
            void from_coo_(COO<COOLabel, COOOrdinal, COOStorage, COOsym, COOut,
                    COOsl, COOme, weighted, COOW, COOWS>& coo) {
                auto coo_copy = coo;
                coo_copy.transpose();

                csc_ = CSC<
                            Label,
                            Ordinal,
                            LabelStorage,
                            OrdinalStorage,
                            weighted,
                            Weight,
                            WeightStorage
                        > { coo_copy };
                coo_copy.free();

                csr_ = CSR<
                            Label,
                            Ordinal,
                            LabelStorage,
                            OrdinalStorage,
                            weighted,
                            Weight,
                            WeightStorage
                        > { coo };
            }

        public:
            /** @brief Initialize from a COO
             *
             * This creates a Matrix from an already-loaded COO.
             *
             * This first copies the COO and transposes the copy, setting
             * that as the CSC.
             *
             * @tparam COOLabel the label for the COO format
             * @tparam COOOrdinal the ordinal for the COO format
             * @tparam COOStorage the storage format of the COO
             * @tparam COOsym whether the COO is symmetrized
             * @tparam COOut whether the COO only keeps the upper triangle
             * @tparam COOsl whether the COO removes self loops
             * @tparam COOme whether the COO removes multiple edges
             * @tparam COOW the weight type of the COO
             * @tparam COOWS the weight storage type of the COO
             * @param coo the COO object to load the CSR from
             */
            template <class COOLabel, class COOOrdinal, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl, bool COOme,
                     class COOW, class COOWS>
            Matrix(COO<COOLabel, COOOrdinal, COOStorage, COOsym, COOut,
                    COOsl, COOme, weighted, COOW, COOWS>& coo) {
                from_coo_(coo);
            }

            /** @brief Build a Matrix from a file
             *
             * @param filename the file to read and load
             */
            Matrix(std::string filename) {
                COO<
                    Label, Ordinal, LabelStorage,
                    false, false, false, false,
                    weighted, Weight, WeightStorage
                > coo {filename};
                from_coo_(coo);
                coo.free();
            }

            /** @brief Free the associated memory */
            void free() {
                csc_.free();
                csr_.free();
            }

            /** @brief Return the number of rows */
            Ordinal nrows() { return csr_.nrows(); }

            /** @brief Return the number of columns */
            Ordinal ncols() { return csr_.ncols(); }

            /** @brief Return the CSR */
            CSR<
                Label,
                Ordinal,
                LabelStorage,
                OrdinalStorage,
                weighted,
                Weight,
                WeightStorage
            >& csr() { return csr_; }

            /** @brief Return the CSC */
            CSC<
                Label,
                Ordinal,
                LabelStorage,
                OrdinalStorage,
                weighted,
                Weight,
                WeightStorage
            >& csc() { return csc_; }

    };

}

#endif
