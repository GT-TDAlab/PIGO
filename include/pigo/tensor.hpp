/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains utilities to read sparse tensor files
 */

#ifndef PIGO_TENSOR_HPP
#define PIGO_TENSOR_HPP

#include <string>
#include <memory>

namespace pigo {

    /** @brief Holds coordinate-addressed tensors
     *
     * A Tensor is designed to read sparse tensor files and expose the
     * resulting file as a coordinate list with weights.
     *
     * @tparam Label the label data type. This type needs to be able to
     *         support the largest coordinate value read inside of the Tensor.
     * @tparam Ordinal the ordinal data type. This type needs to
     *         support large enough values to hold the number of entries
     *         or rows in the Tensor. It defaults to the same type as the
     *         label type.
     * @tparam Storage the storage type of the Tensor. This can either be
     *         vector (std::vector<Label>), a pointer (Label*), or
     *         a shared_ptr (std::shared_ptr<Label>).
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
        bool weighted=true,
        class Weight=float,
        class WeightStorage=Weight*
    >
    class Tensor {
        private:
            /** The coordinate values */
            Storage c_;

            /** The weight values */
            WeightStorage w_;

            /** The order of the tensor */
            Ordinal order_;

            /** The number of entries or non-zeros in the tensor */
            Ordinal m_;

            /** @brief Reads the given file and type into the Tensor
             *
             * @param f the File to read
             * @param ft the FileType of the file. If uknown, AUTO can be
             *        used.
             */
            void read_(File& f, FileType ft);

            /** @brief Reads a tns file into the Tensor
             *
             * This is an internal function that will read an edge list (tns
             * file) to load the Tensor
             *
             * @param r the FileReader to read with
             */
            void read_el_(FileReader& r);

            /** @brief Reads a PIGO binary Tensor
             *
             * @param f the File to read
             */
            void read_bin_(File& f);

            /** @brief Allocate the Tensor
             *
             * Allocates the memory for the Tensor to fit the storage format
             * requested.
             *
             * Note that order_ and m_ must be set before this is called.
             */
            void allocate_();

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
            void read_coord_entry_(size_t &coord_pos, FileReader &r);

            /** @brief Copies values from the other tensor */
            void copy_(const Tensor& other) {
                #pragma omp parallel for
                for (Ordinal pos = 0; pos < order_*m_; ++pos) {
                    Label val = detail::get_value_<
                                Storage,
                                Label
                            >((Storage&)(other.c_), pos);
                    detail::set_value_(c_, pos, val);
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
            /** @brief Initialize a Tensor from a file
             *
             * The file type will attempt to be determined automatically.
             *
             * @param fn the filename to open
             */
            Tensor(std::string fn);

            /** @brief Initialize a Tensor from a file with a specific type
             *
             * @param fn the filename to open
             * @param ft the FileType to use
             */
            Tensor(std::string fn, FileType ft);

            /** @brief Initialize a Tensor from an open File with a specific type
             *
             * @param f the File to use
             * @param ft the FileType to use
             */
            Tensor(File& f, FileType ft);

            /** @brief Initialize an empty Tensor */
            Tensor() : order_(0), m_(0) { }

            /** @brief Provide space for copying in existing, out-of-band data */
            Tensor(Ordinal order, Ordinal m) :
                    order_(order), m_(m) {
                allocate_();
            }

            /** @brief Retrieve the coordinate array
             *
             * @return the coordinate array in the format Storage
             */
            Storage& c() { return c_; }

            /** @brief Retrieve the weight array
             *
             * @return the weight array in the format WeightStorage
             */
            WeightStorage& w() { return w_; }

            /** @brief Retrieves the number of entries in the Tensor
             *
             * @return the count of entries
             */
            Ordinal m() const { return m_; }

            /** @brief Retrieves the order of the tensor
             *
             * @return the order of the tensor
             */
            Ordinal order() const { return order_; }

            /** @brief Saves the Tensor to a binary PIGO file */
            void save(std::string fn);

            /** @brief Write the Tensor out to an ASCII file */
            void write(std::string fn);

            /** @brief Free consumed memory */
            void free() {
                if (m_ > 0) {
                    detail::free_mem_(c_);
                    detail::free_mem_<WeightStorage, weighted>(w_);
                    order_ = 0;
                    m_ = 0;
                }
            }

            /** @brief The copy constructor for creating a new Tensor */
            Tensor(const Tensor& other) : order_(other.order_), m_(other.m_) {
                allocate_();
                copy_(other);
            }

            /** @brief The copy assignment operator */
            Tensor& operator=(const Tensor& other) {
                if (&other != this) {
                    free();
                    order_ = other.order_;
                    m_ = other.m_;
                    allocate_();
                    copy_(other);
                }

                return *this;
            }

            /** The output file header for reading/writing */
            static constexpr const char* tensor_file_header = "PIGO-Tensor-v1";
    };

}

#endif
