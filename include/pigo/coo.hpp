/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains utilities to read and write COO files
 */

#ifndef PIGO_COO_HPP
#define PIGO_COO_HPP

#include <string>
#include <memory>

namespace pigo {

    // We include the prototype here to support converting from CSR
    template<class Label, class Ordinal, class LabelStorage, class OrdinalStorage, bool weighted, class Weight, class WeightStorage>
    class CSR;

    /** @brief Holds coordinate-addressed matrices or graphs
     *
     * A COO is a fundamental object in PIGO. It is able to read a variety
     * of input formats (e.g., matrix market) and exposes the read data as
     * an edge list.
     *
     * The edge list is stored as multiple arrays, one for the x elements
     * and one for the y elements (e.g., src and dst in graphs.) The
     * storage method is a template parameter.
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest value read inside of the COO. In
     *         a graph this is the largest vertex ID.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of entries
     *         or rows in the COO. It defaults to the same type as the
     *         label type.
     * @tparam Storage the storage type of the COO. This can either be
     *         vector (std::vector<Label>), a pointer (Label*), or
     *         a shared_ptr (std::shared_ptr<Label>).
     * @tparam symmetric Ensure that the COO is symmetric, that is ensure
     *         that for every coordinate there is a corresponding
     *         symmetric coordinate with the same value.
     *         If true, this will always be the case. If false, any
     *         symmetry will be based on whether the input is.
     * @tparam keep_upper_triangle_only Only keep values that are in the
     *         upper triangle (if the coordinate is (x,y,val), only keep
     *         it in the coo if x <= y)
     *         If this is used with symmetric, then the COO will first
     *         symmetrize and then remove any value out of the upper
     *         triangle.
     *         If this is used without symmetric, then any edges with
     *         (y > x) will not be included in the COO.
     * @tparam remove_self_loops remove any self loops.
     *         If set to true, this will detect and remove any self loops
     *         (if the coordinate is (x,y,val) and (x==y), the entry will
     *         not be included in the COO.
     * @tparam weighted if true, support and use weights
     * @tparam Weight the weight data type.
     * @tparam WeightStorage the storage type for the weights. This can be
     *         a raw pointer (Weight*), a std::vector
     *         (std::vector<Weight>), or a std::shared_ptr<Weight>.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class Storage=Label*,
        bool symmetric=false,
        bool keep_upper_triangle_only=false,
        bool remove_self_loops=false,
        bool weighted=false,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class COO {
        private:
            /** The X value of the coordinate */
            Storage x_;

            /** The Y value of the coordinate */
            Storage y_;

            /** The weight values */
            WeightStorage w_;

            /** The number of labels in the matrix represented */
            Label n_;

            /** The number of rows */
            Label nrows_;

            /** The number of columns */
            Label ncols_;

            /** The number of entries or non-zeros in the COO */
            Ordinal m_;

            /** @brief Reads the given file and type into the COO
             *
             * @param f the File to read
             * @param ft the FileType of the file. If uknown, AUTO can be
             *        used.
             */
            void read_(File& f, FileType ft);

            /** @brief Reads an edge list into the COO
             *
             * This is an internal function that will read an edge list
             * specific file to load the COO.
             *
             * @param r the FileReader to read with
             */
            void read_el_(FileReader& r);

            /** @brief Reads a matrix market file into the COO
             *
             * This is an internal function that will parse the matrix
             * market header and then load the COO appropriately.
             *
             * @param r the FileReader to read with
             */
            void read_mm_(FileReader& r);

            /** @brief Reads a PIGO binary COO
             *
             * @param f the File to read
             */
            void read_bin_(File& f);

            /** @brief Allocate the COO
             *
             * Allocates the memory for the COO to fit the storage format
             * requested.
             *
             * Note that m_ must be set before this is called.
             */
            void allocate_();

            /** @brief Convert a CSR into this COO
             *
             * @tparam CL the label type of the CSR
             * @tparam CO the ordinal type of the CSR
             * @tparam LStorage the label storage of the CSR
             * @tparam OStorage the ordinal storage of the CSR
             * @tparam CW the weight type of the CSR
             * @tparam CWS the weight storage type of the CSR
             * @param csr the CSR to load from
             */
            template <class CL, class CO, class LStorage, class OStorage, class CW, class CWS>
            void convert_csr_(CSR<CL, CO, LStorage, OStorage, weighted, CW, CWS>& csr);

            /** @brief Read an entry into the appropriate coordinate
             *
             * The position, along with the file reader, will be
             * incremented after reading.
             *
             * @param[in,out] coord_pos the current position in the
             *                coordinate list, e.g., the edge number
             * @param[in,out] r the current file reader
             * @param[in,out] max_row the current maxmium row label seen.
             *                If reading a label that is larger than the
             *                max label, this value will be updated.
             * @param[in,out] max_col the current maxmium col label seen.
             *                If reading a label that is larger than the
             *                max label, this value will be updated.
             * @tparam count_only if true, will not set values and will
             *                only assist in counting by moving through
             *                what would have been read.
             */
            template <bool count_only>
            void read_coord_entry_(size_t &coord_pos, FileReader &r,
                    Label &max_row, Label &max_col);

            /** @brief Copies the COO values from the other COO */
            void copy_(const COO& other) {
                #pragma omp parallel for
                for (Ordinal pos = 0; pos < m_; ++pos) {
                    Label x_val = detail::get_value_<
                                Storage,
                                Label
                            >((Storage&)(other.x_), pos);
                    detail::set_value_(x_, pos, x_val);
                    Label y_val = detail::get_value_<
                                Storage,
                                Label
                            >((Storage&)(other.y_), pos);
                    detail::set_value_(y_, pos, y_val);
                }
                if (detail::if_true_<weighted>()) {
                    #pragma omp parallel for
                    for (Ordinal pos = 0; pos < m_; ++pos) {
                        Weight w_val = detail::get_value_<
                                    WeightStorage,
                                    Weight
                                >((WeightStorage&)(other.w_), pos);
                        detail::set_value_(w_, pos, w_val);
                    }
                }
            }
        public:
            /** @brief Initialize a COO from a file
             *
             * The file type will attempt to be determined automatically.
             *
             * @param fn the filename to open
             */
            COO(std::string fn);

            /** @brief Initialize a COO from a file with a specific type
             *
             * @param fn the filename to open
             * @param ft the FileType to use
             */
            COO(std::string fn, FileType ft);

            /** @brief Initialize a COO from an open File with a specific type
             *
             * @param f the File to use
             * @param ft the FileType to use
             */
            COO(File& f, FileType ft);

            /** @brief Initialize from a CSR
             *
             * @param csr the CSR to convert from
             */
            template<class CL, class CO, typename LabelStorage, typename OrdinalStorage, class CW, class CWS>
            COO(CSR<CL, CO, LabelStorage, OrdinalStorage, weighted, CW, CWS>& csr);

            /** @brief Initialize an empty COO */
            COO() : n_(0), nrows_(0), ncols_(0), m_(0) { }

            /** @brief Provide space for copying in existing, out-of-band data */
            COO(Label n, Label nrows, Label ncols, Ordinal m) :
                    n_(n), nrows_(nrows), ncols_(ncols), m_(m) {
                allocate_();
            }

            /** @brief Retrieve the X coordinate array
             *
             * @return the X array in the format Storage
             */
            Storage& x() { return x_; }

            /** @brief Retrieve the Y coordinate array
             *
             * @return the Y array in the format Storage
             */
            Storage& y() { return y_; }

            /** @brief Retrieve the weight array
             *
             * @return the weight array in the format WeightStorage
             */
            WeightStorage& w() { return w_; }

            /** @brief Retrieves the number of entries in the COO
             *
             * @return the count of entries
             */
            Ordinal m() { return m_; }

            /** @brief Retrieves the number of labels the COO contains
             *
             * Note: This will include any zero entry-labels. So, the
             * count is the largest seen label+1.
             *
             * @return the number of labels
             */
            Label n() { return n_; }

            /** @brief Update the largest label */
            void set_n(Label new_n) { n_ = new_n; }

            /** @brief Update the number of rows in the matrix */
            void set_nrows(Label new_nrows) { nrows_ = new_nrows; }

            /** @brief Update the number of cols in the matrix */
            void set_ncols(Label new_ncols) { ncols_ = new_ncols; }

            /** @brief Retrieves the number of rows in the COO
             *
             * @return the number of rows
             */
            Label nrows() { return nrows_; }

            /** @brief Retrieves the number of columns in the COO
             *
             * @return the number of columns
             */
            Label ncols() { return ncols_; }

            /** @brief Saves the COO to a binary PIGO file */
            void save(std::string fn);

            /** @brief Write the COO out to an ASCII file */
            void write(std::string fn);

            /** @brief Utility to free consumed memory
             *
             * As an IO library, PIGO generally leaves memory cleanup to
             * downstream applications and does not always deallocate in
             * destructors. In some cases it is helpful for PIGO to
             * cleanup directly and then this can be used.
             */
            void free() {
                if (m_ > 0) {
                    detail::free_mem_(x_);
                    detail::free_mem_(y_);
                    detail::free_mem_<WeightStorage, weighted>(w_);
                    m_ = 0;
                }
            }

            /** @brief The copy constructor for creating a new COO */
            COO(const COO& other) : n_(other.n_), nrows_(other.nrows_),
                    ncols_(other.ncols_), m_(other.m_) {
                allocate_();
                copy_(other);
            }

            /** @brief The copy assignment operator */
            COO& operator=(const COO& other) {
                if (&other != this) {
                    free();
                    n_ = other.n_;
                    nrows_ = other.nrows_;
                    ncols_ = other.ncols_;
                    m_ = other.m_;
                    allocate_();
                    copy_(other);
                }

                return *this;
            }

            /** @brief Transpose the COO, swapping x and y */
            COO& transpose() {
                std::swap(x_, y_);
                return *this;
            }

            /** The output file header for reading/writing */
            static constexpr const char* coo_file_header = "PIGO-COO-v1";
    };

    /** @brief Holds weighted coordinate-addressed matrices or graphs
     *
     * WCOO is a wrapper around COO that is able to hold weights alongside
     * the coordinates. This is used either with weighted graphs or
     * non-binary matrices.
     *
     * This differs from COO by always setting the weighted flag.
     * For template parameter deatils, see COO.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class Storage=Label*,
        class Weight=float,
        class WeightStorage=Weight*,
        bool symmetric=false,
        bool keep_upper_triangle_only=false,
        bool remove_self_loops=false
    >
    using WCOO = COO<Label, Ordinal, Storage,
        symmetric, keep_upper_triangle_only, remove_self_loops,
        true, Weight, WeightStorage>;

    /** @brief Holds a pointer based weighted COO
     *
     * For template parameters, please see COO.
     */
    template<
        class Label=uint32_t,
        class Ordinal=Label,
        class Weight=float,
        bool symmetric=false,
        bool keep_upper_triangle_only=false,
        bool remove_self_loops=false
    >
    using WCOOPtr = COO<
        Label,
        Ordinal,
        Label*,
        symmetric,
        keep_upper_triangle_only,
        remove_self_loops,
        true,
        Weight,
        Weight*>;

}

#endif
