/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains utilities to read and write CSR files
 */

#ifndef PIGO_CSR_HPP
#define PIGO_CSR_HPP

namespace pigo {

    /** @brief Holds compressed sparse row matrices or graphs
     *
     * This is a fundamental object in PIGO. It is used to represent
     * sparse matrices and graphs. It can be loaded directly from
     * files that are in adjacency formats, such as CHACO/METIS files.
     *
     * In many cases, this is the desired format for a graph or matrix in
     * memory. When sparse graphs or matrices are delivered in COO
     * formats, such as matrix market or edge lists, they are frequently
     * converted to CSR. This class can automatically handle such
     * conversions internally.
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest value read inside of the COO. In
     *         a graph this is the largest vertex ID.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of endpoints
     *         or rows in the COO. It defaults to the same type as the
     *         label type.
     * @tparam LabelStorage the storage type of the endpoints of the CSR.
     *         This can either be vector (std::vector<Label>),
     *         a pointer (Label*), or a shared_ptr
     *         (std::shared_ptr<Label>).
     * @tparam OrdinalStorage the storage type of the offsets of the CSR.
     *         This can either be vector (std::vector<Ordinal>),
     *         a pointer (Ordinal*), or a shared_ptr
     *         (std::shared_ptr<Ordinal>).
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type. This type needs to be able to
     *         support the largest value read inside of the COO. In
     *         a graph this is the largest vertex ID.
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
    class CSR {
        protected:
            /** The endpoints hold the labels (e.g., representing edges) */
            LabelStorage endpoints_;

            /** The offsets into the endpoints */
            OrdinalStorage offsets_;

            /** The weight values */
            WeightStorage weights_;

        private:
            /** The number of labels */
            Label n_;

            /** The number of endpoints */
            Ordinal m_;

            /** The number of rows */
            Label nrows_;

            /** The number of columns */
            Label ncols_;

            /** @brief Read the CSR from the given file and format
             *
             * @param f the File to read from
             * @param ft the FileFormat to use to read
             */
            void read_(File& f, FileType ft);

            /** @brief Read a binary CSR from disk
             *
             * This is an internal function that will populate the CSR
             * from a binary PIGO file.
             *
             * @param f the File to read from
             */
            void read_bin_(File& f);

            /** @brief Read a GRAPH file format
             *
             * This is an internal function that will load a GRAPH file
             * to build the CSR.
             *
             * @param r the FileReader to load from
             */
            void read_graph_(FileReader& r);

            /** @brief Allocate the storage for the CSR */
            void allocate_();

            /** @brief Convert a COO into this CSR
             *
             * @tparam COOLabel the label for the COO format
             * @tparam COOOrdinal the ordinal for the COO format
             * @tparam COOStorage the storage format of the COO
             * @tparam COOsym whether the COO is symmetrized
             * @tparam COOut whether the COO only keeps the upper triangle
             * @tparam COOsl whether the COO removes self loops
             * @tparam COOW the weight type of the COO
             * @tparam COOWS the weight storage type of the COO
             * @param coo the COO to load from
             */
            template <class COOLabel, class COOOrdinal, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl,
                     class COOW, class COOWS>
            void convert_coo_(COO<COOLabel, COOOrdinal, COOStorage,
                    COOsym, COOut, COOsl, weighted, COOW, COOWS>&
                    coo);
        public:
            /** @brief Initialize an empty CSR */
            CSR() : n_(0), m_(0), nrows_(0), ncols_(0) { }

            /** @brief Allocate a CSR for the given size */
            CSR(Label n, Ordinal m, Label nrows, Label ncols) :
                    n_(n), m_(m), nrows_(nrows), ncols_(ncols) {
                allocate_();
            }

            /** @brief Initialize from a COO
             *
             * This creates a CSR from an already-loaded COO.
             *
             * Note that this will densely fill in all labels, so if there
             * are many empty rows there will be unnecessary space used.
             *
             * @tparam COOLabel the label for the COO format
             * @tparam COOOrdinal the ordinal for the COO format
             * @tparam COOStorage the storage format of the COO
             * @tparam COOsym whether the COO is symmetrized
             * @tparam COOut whether the COO only keeps the upper triangle
             * @tparam COOsl whether the COO removes self loops
             * @tparam COOW the weight type of the COO
             * @tparam COOWS the weight storage type of the COO
             * @param coo the COO object to load the CSR from
             */
            template <class COOLabel, class COOOrdinal, class COOStorage,
                     bool COOsym, bool COOut, bool COOsl,
                     class COOW, class COOWS>
            CSR(COO<COOLabel, COOOrdinal, COOStorage, COOsym, COOut,
                    COOsl, weighted, COOW, COOWS>& coo);

            /** @brief Initialize from a file
             *
             * The file type will attempt to be determined automatically.
             *
             * @param fn the filename to open
             */
            CSR(std::string fn);

            /** @brief Initialize from a file with a specific type
             *
             * @param fn the filename to open
             * @param ft the FileType to use
             */
            CSR(std::string fn, FileType ft);

            /** @brief Initialize from an open file with a specific type
             *
             * @param f the open File
             * @param ft the FileType to use
             */
            CSR(File& f, FileType ft);

            /** @brief Return the endpoints
             *
             * @return the endpoints in the LabelStorage format
             */
            LabelStorage& endpoints() { return endpoints_; }

            /** @brief Return the offsets
             *
             * These contain Ordinals that show the offset for the current
             * label into the endpoints. These are not pointers directly.
             *
             * @return the offsets in the OrdinalStorage format
             */
            OrdinalStorage& offsets() { return offsets_; }

            /** @brief Return the weights, if available
             *
             * This returns the WeightStorage for the weights, if the CSR
             * is weighted.
             *
             * @return the weights in the WeightStorage format
             */
            WeightStorage& weights() { return weights_; }

            /** @brief Retrieves the number of endpoints in the CSR
             *
             * @return the count of endpoints
             */
            Ordinal m() const { return m_; }

            /** @brief Retrieves the number of labels the CSR contains
             *
             * Note that this includes labels with no endpoints.
             *
             * @return the number of labels
             */
            Label n() const { return n_; }

            /** @brief Retrieves the number of rows in the CSR
             *
             * @return the number of rows
             */
            Label nrows() const { return nrows_; }

            /** @brief Retrieves the number of columns in the CSR
             *
             * @return the number of columns
             */
            Label ncols() const { return ncols_; }

            /** @brief Sort all row adjacencies in the CSR */
            void sort();

            /** @brief Return a new CSR without duplicate entries */
            template<class nL=Label, class nO=Ordinal, class nLS=LabelStorage, class nOS=OrdinalStorage, bool nw=weighted, class nW=Weight, class nWS=WeightStorage>
            CSR<nL, nO, nLS, nOS, nw, nW, nWS> new_csr_without_dups();

            /** @brief Utility to free consumed memory
             *
             * As an IO library, PIGO generally leaves memory cleanup to
             * downstream applications and does not always deallocate in
             * destructors. In some cases it is helpful for PIGO to
             * cleanup directly and then this can be used.
             */
            void free() {
                detail::free_mem_(endpoints_);
                detail::free_mem_(offsets_);
                detail::free_mem_<WeightStorage, weighted>(weights_);
            }

            /** @brief Return the size of the binary save file
             *
             * @return size_t containing the binary file size
             */
            size_t save_size () const;

            /** @brief Save the loaded CSR as a PIGO binary file
             *
             * This saves the current CSR to disk
             *
             * @param fn the filename to save as
             */
            void save(std::string fn);

            /** @brief Save the loaded CSR as a PIGO binary file
             *
             * This saves the current CSR to an open file
             *
             * @param w the File to save to
             */
            void save(File& w);

            /** The output file header for reading/writing */
            static constexpr const char* csr_file_header = "PIGO-CSR-v2";
    };

    /** @brief Holds a pointer based weighted CSR
     *
     * For template parameters, please see CSR.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class Weight=float
    >
    using WCSRPtr = CSR<
        Label,
        Ordinal,
        Label*,
        Ordinal*,
        true,
        Weight,
        Weight*>;

    /** @brief A compressed sparse column representation
     *
     * This is a transposed CSR.
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest value read inside of the CSR. In
     *         a graph this is the largest vertex ID.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of endpoints
     *         or rows in the CSR. It defaults to the same type as the
     *         label type.
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
    class CSC : public CSR<
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


}

#endif
