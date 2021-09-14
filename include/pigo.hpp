/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 *
 * This file contains general PIGO input and output functions, used
 * throughout the remainder of the library.
 */

#ifndef PIGO_HPP
#define PIGO_HPP

#include <stdexcept>
#include <memory>
#include <vector>
#include <string>

namespace pigo {
    /** @brief Thrown by errors detected in PIGO */
    class Error : public ::std::runtime_error {
        public:
            template<class T>
            Error(T t) : ::std::runtime_error(t) { }
    };

    /** @brief Thrown when something is not implemented yet */
    class NotYetImplemented : public Error {
        public:
            template<class T>
            NotYetImplemented(T t) : Error(t) { }
    };

    /** Support for detecting vectors */
    template <typename T> struct is_vector:std::false_type{};
    template <typename... Args> struct is_vector<std::vector<Args...>>:std::true_type{};

    /** Support for detected shared pointers */
    template <typename T> struct is_sptr:std::false_type{};
    template <typename... Args> struct is_sptr<std::shared_ptr<Args...>>:std::true_type{};

    /** @brief Keep track of the state of an open file */
    typedef const char* FilePos;
    /** @brief Keep track of the state of an open writeable file */
    typedef char* WFilePos;

    /** @brief Performs actions on an opened PIGO file */
    class FileReader {
        private:
            /** Keep track of the end of the file */
            FilePos end;
        public:
            /** Keep track of the current position */
            FilePos d;

            /** @brief Initialize a new FileReader
             *
             * @param d the current position of the reader
             * @param end one beyond the last valid position
             */
            FileReader(FilePos d, FilePos end) :
                end(end), d(d) { }

            /** @brief Move the reader pass any comment lines */
            void skip_comments();

            /** @brief Read an integer from the file
            *
            * Note: this reads integers in base 10 only
            *
            * @return the newly read integer
            */
            template<typename T>
            T read_int();

            /** @brief Read a floating point value from the file
            *
            * @return the newly read floating point
            */
            template<typename T>
            T read_fp();

            /** @brief Read the sign value from an integer
             *
             * Reads out either + or -, as appropriate, from the given file
             * position. The file position is the incremented.
             *
             * @tparam T the type of the integer sign to return
             * @return a T value of either 1 or -1 as appropriate
             */
            template<typename T>
            T read_sign() {
                if (*d == '-') return (T)(-1);
                return (T)1;
            }

            /** @brief Determine if only spaces remain before the line
            *
            * Note that this does not increment the current position.
            *
            * @return true if there are only spaces, false otherwise
            */
            bool at_end_of_line();

            /** @brief Move the reader to the next non-int */
            void move_to_non_int();

            /** @brief Move the reader to the next non-floating point */
            void move_to_non_fp();

            /** @brief Move the reader to the next floating point */
            void move_to_fp();

            /** @brief Move to the first integer
             *
             * This will move to the first found integer. If it is already
             * on an integer, it will not move.
             */
            void move_to_first_int();

            /** @brief Move to the next integer
            *
            * Note: this will move through the current integer and then through
            * any non-integer character to get to the next integer
            */
            void move_to_next_int();

            /** @brief Move to the next signed integer
            *
            * Note: this will move through the current integer and then through
            * any non-integer character to get to the next integer
            *
            * These integers are signed and so can start with + or -
            */
            void move_to_next_signed_int();

            /** @brief Move to the next integer or newline
            *
            * Note: this will move through the current integer or newline
            * and then through any other character to get to the next
            * integer or newline
            */
            void move_to_next_int_or_nl();

            /** @brief Move to the end of the current line
             *
             * This moves through the line and finishes on the newline
             * character itself.
             */
            void move_to_eol();

            /** @brief Increment the file reader by a count */
            FileReader operator+(size_t s) {
                FileReader n {d, end};
                n.d = d + s;
                n.end = end;
                if (n.d > end) n.d = end;
                return n;
            }
            /** @brief Increment the file reader by a count */
            FileReader& operator+=(size_t s) {
                d += s;
                if (d > end) d = end;
                return *this;
            }

            /** @brief Set an additional, smaller end
             *
             * This will either set a new end, if it is smaller than the
             * current, or do nothing.
             *
             * @param new_end the new end to consider
             */
            void smaller_end(FileReader &rhs) {
                if (rhs.d < end) end = rhs.d;
            }

            /** @brief Return if the reader is able to read
             *
             * @return true if there are values to read, false otherwise
             */
            bool good() { return d < end; }

            /** @brief Return the remaining size to be read
             *
             * @return the number of bytes remaining
             */
            size_t size() { return end - d; }

            /** @brief Check whether the reader is at the string
             *
             * @param s the string to check against
             * @return true if the string matches, false otherwise
             */
            bool at_str(std::string s);

            /** @brief Return the current character of the reader
             *
             * @return character the reader is at
             */
            char peek() {
                if (d == end) return 0;
                return *d;
            }

            /** @brief Return whether the reader is at a newline or EOL
             *
             * @return true if the reader is at the end of a line, false
             *         otherwise
             */
            bool at_nl_or_eol() {
                if (d == end) return true;
                return (*d == '\n' || *d == '%' || *d == '#');
            }

            /** @brief Return whether the reader is at a '0' integer
             *
             * @return true if the reader is at a '0' integer
             */
            bool at_zero() {
                if (d >= end+1) return false;
                if (*d != '0') return false;
                if (*(d+1) >= '0' && *(d+1) <= '9') return false;
                return true;
            }
    };

    /** @brief Contains the supported file types */
    enum FileType {
        /** An edge list with size, typically .mtx files */
        MATRIX_MARKET,
        /** A file where each line is an edge. This is the simplest format
         * PIGO supports. Each line is `src dst` if it is unweighted, or
         * `src dst weight` if weighted. */
        EDGE_LIST,
        /** A binary format storing a PIGO COO */
        PIGO_COO_BIN,
        /** A binary format storing a PIGO CSR */
        PIGO_CSR_BIN,
        /** A binary format storing a PIGO DiGraph */
        PIGO_DIGRAPH_BIN,
        /** A file with a head and where each line contains an adjacency
         * list */
        GRAPH,
        /** A special format where PIGO will try to detect the input */
        AUTO
    };

    /** @brief The support PIGO file opening modes
     *
     * READ indicates read-only, whereas WRITE indicates read and write,
     * but on opening will create a new file (removing any old file.)
     * */
    enum OpenMode {
        /** Indicates a read-only file */
        READ,
        /** Indicates a writeable file, existing files will be removed */
        WRITE
    };

    /** @brief Manages a file opened for parallel access */
    class File {
        protected:
            /** Contains the data of the file and can be access in parallel */
            char* data_;
            /** Total size of the allocated data memory block */
            size_t size_;
            /** The current file position */
            FilePos fp_;
            /** The filename */
            std::string fn_;
        public:
            /** @brief Opens the given file
             *
             * @param fn the file name to open
             * @param mode the mode to open the file in (READ, WRITE)
             * @param max_size (only used when mode=WRITE) the maximum
             *        size to allocate for the file
             */
            File(std::string fn, OpenMode mode, size_t max_size=0);

            /** @brief Closes the open file and removes related memory */
            ~File() noexcept;

            /** @brief Copying File is unavailable */
            File(const File&) = delete;

            /** @brief Copying File is unavailable */
            File& operator=(const File&) = delete;

            /** @brief Move constructor */
            File(File &&o) {
                data_ = o.data_;
                size_ = o.size_;
            }

            /** @brief Move operator */
            File& operator=(File&& o);

            /** @brief Return the current file position object */
            FilePos fp() { return fp_; }

            /** @brief Seek to the specified offset in the file
             *
             * @param pos the position to seek to
             */
            void seek(size_t pos);

            /** @brief Read the next value from the file
             *
             * @tparam T the type of object to read
             *
             * @return the resulting object
             */
            template<class T> T read();

            /** @brief Read passed the given string
             *
             * This will ensure that the string exists in the file and
             * then read passed it. The file position will be at the first
             * character after the string when it is done. An error will
             * be throw if the string does not match.
             *
             * @param s the string to compare and read
             */
            void read(const std::string& s);

            /** @brief Write the given value to the file
             *
             * @tparam T the type of object to write
             * @param val the value to write
             *
             * @return the resulting object
             */
            template<class T> void write(T val);

            /** @brief Write a binary region in parallel
            *
            * @param v the region of data to write
            * @param v_size the size of the region of data to write
            */
            void parallel_write(char* v, size_t v_size);

            /** @brief Read a binary region in parallel
            *
            * @param v the region of data to save to
            * @param v_size the size of the region of data to read
            */
            void parallel_read(char* v, size_t v_size);

            /** @brief Return the size of the file */
            size_t size() { return size_; }

            /** @brief Auto-detect the file type
             *
             * This will determine the file type based on a mixture of the
             * extension, the contents of the file, and investigating the
             * structure of the lines.
             *
             * Note that this is a best-guess.
             *
             * @return the determined FileType
             */
            FileType guess_file_type();

            /** @brief Return a FileReader for this file
             *
             * @return returns a new FileReader object for this file
             */
            FileReader reader() {
                return FileReader {fp_, data_+size_};
            }
    };
    /** @brief Opens a read-only file for use in PIGO */
    class ROFile : public File {
        public:
            /** @brief Opens the given file
             *
             * @param fn the file name to open
             */
            ROFile(std::string fn) : File(fn, READ) { }
    };
    /** @brief Opens a writeable file for use in PIGO */
    class WFile : public File {
        public:
            /** @brief Opens the given file
             *
             * @param fn the file name to open
             * @param max_size the size to allocate for the file
             */
            WFile(std::string fn, size_t max_size) :
                File(fn, WRITE, max_size) { }
    };

    /** @brief Read a binary value from an open file
     *
     * Reads a binary value out from the given file position. The file
     * position object is incremented to just passed the value read out.
     *
     * @tparam T the type of object to read
     * @param[in,out] fp the file position object of the open file
     *
     * @return the value read out
     */
    template<class T> T read(FilePos &fp);

    /** @brief Write a binary value from an open file
     *
     * Writes a binary value into a file at the given position.
     * The file position object is incremented appropriately.
     *
     * @tparam T the type of object to write
     * @param[in,out] fp the file position object of the open file
     * @param val the value to write to the file
     */
    template<class T> void write(FilePos &fp, T val);

    /** @brief Return the size taken to write the given object
     *
     * @tparam T the type of the object
     * @param obj the object to return the output size
     *
     * @return size_t number of bytes used to write the object
     */
    template<typename T,
        typename std::enable_if<!std::is_integral<T>::value, bool>::type = false,
        typename std::enable_if<!std::is_floating_point<T>::value, bool>::type = false
        > inline size_t write_size(T obj);
    template<typename T,
        typename std::enable_if<std::is_integral<T>::value, bool>::type = true,
        typename std::enable_if<!std::is_floating_point<T>::value, bool>::type = false
        > inline size_t write_size(T obj);
    template<typename T,
        typename std::enable_if<!std::is_integral<T>::value, bool>::type = false,
        typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true
        > inline size_t write_size(T obj);

    /** @brief Write an ASCII value to an open file
     *
     * @tparam T the type of object to write
     * @param[in,out] fp the file position of the open file
     * @param obj the object to write
     */
    template<typename T,
        typename std::enable_if<!std::is_integral<T>::value, bool>::type = false,
        typename std::enable_if<!std::is_floating_point<T>::value, bool>::type = false
        > inline void write_ascii(FilePos &fp, T obj);
    template<typename T,
        typename std::enable_if<std::is_integral<T>::value, bool>::type = true,
        typename std::enable_if<!std::is_floating_point<T>::value, bool>::type = false
        > inline void write_ascii(FilePos &fp, T obj);
    template<typename T,
        typename std::enable_if<!std::is_integral<T>::value, bool>::type = false,
        typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true
        > inline void write_ascii(FilePos &fp, T obj);

    namespace detail {

        /** A holder for allocation implementations */
        template<bool do_alloc, typename T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct allocate_impl_ {
            static void op_(T&, size_t) {
                throw Error("Invalid allocation strategy");
            }
        };

        /** The implementation that will not allocate */
        template<typename T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct allocate_impl_<false, T, ptr_flag, vec_flag, sptr_flag> {
            static void op_(T&, size_t) { }
        };

        /** The raw pointer allocation implementation */
        template<typename T>
        struct allocate_impl_<true, T, true, false, false> {
            static void op_(T& it, size_t nmemb) {
                it = static_cast<T>(malloc(sizeof(*(T){nullptr})*nmemb));
                if (it == NULL)
                    throw Error("Unable to allocate");
            }
        };

        /** The vector allocation implementation */
        template<typename T>
        struct allocate_impl_<true, T, false, true, false> {
            static void op_(T& it, size_t nmemb) {
                it.resize(nmemb);
            }
        };

        /** The shared_ptr allocation implementation */
        template<typename T>
        struct allocate_impl_<true, T, false, false, true> {
            static void op_(T& it, size_t nmemb) {
                it = std::shared_ptr<typename T::element_type>(
                        new typename T::element_type[nmemb],
                        std::default_delete<typename T::element_type []>()
                        );
            }
        };

        /** @brief Allocates the given item appropriately
        *
        * @tparam T the storage type
        * @tparam do_alloc whether to allocate or not
        * @param[out] it the item that will be allocated
        * @param nmemb the number of members to allocate
        */
        template<class T, bool do_alloc=true>
        inline
        void allocate_mem_(T& it, size_t nmemb) {
            // Use the appropriate allocation strategy
            allocate_impl_<do_alloc, T,
                std::is_pointer<T>::value,
                is_vector<T>::value,
                is_sptr<T>::value
            >::op_(it, nmemb);
        }

        /** A holder for freeing implementations */
        template<bool do_free, typename T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct free_impl_ {
            static void op_(T&) { };
        };

        /** The raw pointer allocation implementation */
        template<typename T>
        struct free_impl_<true, T, true, false, false> {
            static void op_(T& it) {
                free(it);
            }
        };

        /** The vector allocation implementation */
        template<typename T>
        struct free_impl_<true, T, false, true, false> {
            static void op_(T&) { }
        };

        /** The shared_ptr allocation implementation */
        template<typename T>
        struct free_impl_<true, T, false, false, true> {
            static void op_(T&) { }
        };

        /** @brief Frees the allocated item if the template parameter is true
        *
        * @tparam T the storage type
        * @tparam do_free whether to free the storage item or not
        * @param[out] it the item that will be allocated
        */
        template<class T, bool do_free=true>
        inline
        void free_mem_(T& it) {
            // Use the appropriate allocation strategy
            free_impl_<do_free, T,
                std::is_pointer<T>::value,
                is_vector<T>::value,
                is_sptr<T>::value
            >::op_(it);
        }

        /** The raw data retrieval implementation */
        template<typename T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct get_raw_data_impl_;

        /** The raw pointer implementation */
        template<typename T>
        struct get_raw_data_impl_<T, true, false, false> {
            static char* op_(T& v) { return (char*)(v); }
        };

        /** The vector implementation */
        template<typename T>
        struct get_raw_data_impl_<T, false, true, false> {
            static char* op_(T& v) { return (char*)(v.data()); }
        };

        /** The shared_ptr implementation */
        template<typename T>
        struct get_raw_data_impl_<T, false, false, true> {
            static char* op_(T& v) { return (char*)(v.get()); }
        };

        /** @brief Returns a pointer to the raw data in an allocation
        *
        * @tparam T the storage type
        * @param v the storage item to get the raw data pointer of
        * @return a char pointer to the raw data
        */
        template<class T>
        inline
        char* get_raw_data_(T& v) {
            return get_raw_data_impl_<T,
                std::is_pointer<T>::value,
                is_vector<T>::value,
                is_sptr<T>::value
            >::op_(v);
        }

        /** A holder for the storage location setting implementation */
        template<class S, class T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct set_value_impl_;

        /** The raw pointer value setting implementation */
        template<class S, class T>
        struct set_value_impl_<S, T, true, false, false> {
            static void op_(S &space, size_t offset, T val) {
                space[offset] = val;
            }
        };

        /** The vector value setting implementation */
        template<class S, class T>
        struct set_value_impl_<S, T, false, true, false> {
            static void op_(S &space, size_t offset, T val) {
                space[offset] = val;
            }
        };

        /** The shared_ptr value setting implementation */
        template<class S, class T>
        struct set_value_impl_<S, T, false, false, true> {
            static void op_(S &space, size_t offset, T val) {
                space.get()[offset] = val;
            }
        };

        /** @brief Set the given storage location to the specified value
        *
        * @param space the space to set a value in (raw pointer, vector,
        *        smart pointer)
        * @param offset the offset to set the value at
        * @param value the value to set
        */
        template<class S, class T>
        inline
        void set_value_(S &space, size_t offset, T val) {
            // Use the appropriate strategy based on the space type
            set_value_impl_<S, T,
                std::is_pointer<S>::value,
                is_vector<S>::value,
                is_sptr<S>::value
            >::op_(space, offset, val);
        }

        /** A holder for the getting storage implementation */
        template<class S, class T, bool ptr_flag, bool vec_flag, bool sptr_flag>
        struct get_value_impl_;

        /** The raw pointer get implementation */
        template<class S, class T>
        struct get_value_impl_<S, T, true, false, false> {
            static T op_(S &space, size_t offset) {
                return space[offset];
            }
        };

        /** The vector get implementation */
        template<class S, class T>
        struct get_value_impl_<S, T, false, true, false> {
            static T op_(S &space, size_t offset) {
                return space[offset];
            }
        };

        /** The shared_ptr get implementation */
        template<class S, class T>
        struct get_value_impl_<S, T, false, false, true> {
            static T op_(S &space, size_t offset) {
                return space.get()[offset];
            }
        };

        /** @brief Set the given storage location to the specified value
        *
        * @tparam S the storage type
        * @tparam T the underlying value type
        * @param space the space to get a value in (raw pointer, vector,
        *        smart pointer)
        * @param offset the offset to get the value at
        */
        template<class S, class T>
        inline
        T get_value_(S &space, size_t offset) {
            // Use the appropriate strategy based on the space type
            return get_value_impl_<S, T,
                std::is_pointer<S>::value,
                is_vector<S>::value,
                is_sptr<S>::value
            >::op_(space, offset);
        }

        /** @brief Implementation for false template parameters */
        template <bool B>
        struct if_true_i_ {
            static bool op_() { return false; }
        };
        /** @brief Implementation for true template parameters */
        template <>
        struct if_true_i_<true> {
            static bool op_() { return true; }
        };
        /** @brief Structure that returns bool template parameters
         *
         * @tparam B the boolean template parameter
         */
        template <bool B>
        bool if_true_() {
            return if_true_i_<B>::op_();
        }

    }

    /** @brief Write a binary region in parallel
     *
     * @param fp the FilePos to begin writing at. This will be incremented
     *        passed the written block
     * @param v the region of data to write
     * @param v_size the size of the region of data to write
     */
    void parallel_write(FilePos &fp, char* v, size_t v_size);

    /** @brief Read a binary region in parallel
     *
     * @param fp the FilePos to begin writing at. This will be incremented
     *        passed the read region
     * @param v the region of data to save to
     * @param v_size the size of the region of data to read
     */
    void parallel_read(FilePos &fp, char* v, size_t v_size);

}

// Load the rest of PIGO
#include "pigo/coo.hpp"
#include "pigo/csr.hpp"
#include "pigo/matrix.hpp"
#include "pigo/graph.hpp"

// Load the implementations
#include "pigo/impl/pigo.impl.hpp"
#include "pigo/impl/coo.impl.hpp"
#include "pigo/impl/csr.impl.hpp"
#include "pigo/impl/graph.impl.hpp"

#endif /* PIGO_HPP */
