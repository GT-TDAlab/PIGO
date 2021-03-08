/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 */

#include <cmath>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <omp.h>

namespace pigo {
    inline
    File::File(std::string fn, OpenMode mode, size_t max_size) :
                fn_(fn) {
        int open_mode = O_RDONLY;
        int prot = PROT_READ;
        char fopen_mode[] = "rb";
        if (mode == WRITE) {
            open_mode = O_RDWR;
            prot = PROT_WRITE | PROT_READ;
            fopen_mode[1] = '+';
            if (max_size == 0)
                throw Error("max_size is too low to write");
        } else if (max_size > 0)
            throw Error("Max_size is only used for writing");

        if (mode == WRITE) {
            // Set the file to the given size
            FILE *w_f = fopen(fn.c_str(), "w");
            if (w_f == NULL) throw Error("PIGO: Unable to open file for writing");
            if (fseeko(w_f, max_size-1, SEEK_SET) != 0) throw Error("PIGO: Seek to set size");
            if (fputc(1, w_f) != 1) throw Error("PIGO: Unable to set size");
            if (fclose(w_f) != 0) throw Error("PIGO: Unable to close new file");
        }

        // Open the file to get the total size and base the mmap on
        #ifdef _GNU_SOURCE
        int fd = open(fn.c_str(), open_mode | O_DIRECT);
        #else
        int fd = open(fn.c_str(), open_mode);
        #ifdef __APPLE__
        fcntl(fd, F_NOCACHE, 1);
        #endif
        #endif
        if (fd < 0) throw Error("Unable to open file");
        FILE *f = fdopen(fd, fopen_mode);
        if (f == NULL) throw Error("PIGO: fdopen file");

        // Find the file size
        if (fseeko(f, 0 , SEEK_END) != 0)  throw Error("PIGO: Unable to seek to end");
        size_ = ftello(f);
        if (size_ == (size_t)(-1)) throw Error("PIGO: Invalid size");

        if (mode == WRITE && size_ != max_size)
            throw Error("PIGO: Wrong file size of new file");

        // MMAP the space
        data_ = (char*)mmap(NULL, size_*sizeof(char), prot,
                MAP_SHARED | MAP_NORESERVE, fd, 0);
        if (data_ == MAP_FAILED) throw Error("PIGO: MMAP");
        if (fclose(f) != 0) throw Error("PIGO: Fclose");

        // Advise the mmap for performance
        if (madvise(data_, size_, MADV_WILLNEED) != 0) throw Error("PIGO: madvise");

        // Finally, set the file position
        seek(0);
    }

    inline
    File::~File() noexcept {
        if (data_) {
            munmap(data_, size_);
            data_ = nullptr;
        }
    }

    inline
    File& File::operator=(File&& o) {
        if (&o != this) {
            if (data_)
                munmap(data_, size_);
            data_ = o.data_;
            size_ = o.size_;
            o.data_ = nullptr;
        }
        return *this;
    }

    template<class T>
    inline
    T File::read() {
        return ::pigo::read<T>(fp_);
    }

    inline
    void File::read(const std::string& s) {
        FileReader r = reader();
        if (!r.at_str(s)) throw Error("Cannot read the given string");
        // Move passed it
        fp_ += s.size();
    }

    template<class T>
    inline
    void File::write(T val) {
        return ::pigo::write(fp_, val);
    }

    inline
    void File::parallel_write(char* v, size_t v_size) {
        ::pigo::parallel_write(fp_, v, v_size);
    }

    inline
    void File::parallel_read(char* v, size_t v_size) {
        ::pigo::parallel_read(fp_, v, v_size);
    }

    inline
    void File::seek(size_t pos) {
        if (pos >= size_) throw Error("seeking beyond end of file");
        fp_ = data_ + pos;
    }

    inline
    FileType File::guess_file_type() {
        // First, check for a PIGO header
        FileReader r = reader();
        if (r.at_str(COO<>::coo_file_header))
            return PIGO_COO_BIN;
        if (r.at_str(CSR<>::csr_file_header))
            return PIGO_CSR_BIN;
        if (r.at_str("PIGO"))
            throw Error("Unsupported PIGO binary format, likely version mismatch");
        // Check the filename for .mtx
        std::string ext_mtx { ".mtx" };
        if (fn_.size() >= ext_mtx.size() &&
                fn_.compare(fn_.size() - ext_mtx.size(), ext_mtx.size(), ext_mtx) == 0)
            return MATRIX_MARKET;

        std::string ext_g { ".graph" };
        if (fn_.size() >= ext_g.size() &&
                fn_.compare(fn_.size() - ext_g.size(), ext_g.size(), ext_g) == 0)
            return GRAPH;
        // In future version, we can add a simple CSR-like file check by
        // looking at a few lines and counting elements
        // Default to an edge list
        return EDGE_LIST;
    };

    template<class T>
    inline
    T read(FilePos &fp) {
        T res = *(T*)fp;
        fp += sizeof(T);
        return res;
    }

    template<class T>
    inline
    void write(FilePos &fp, T val) {
        *(T*)fp = val;
        fp += sizeof(T);
    }

    template<>
    inline
    void write(FilePos &fp, std::string val) {
        for (char x : val) {
            write(fp, x);
        }
    }

    inline
    void FileReader::skip_comments() {
        while (d < end && (*d == '%' || *d == '#'))
            while (d < end && (*d++ != '\n')) { }
    }

    template<typename T>
    inline
    T FileReader::read_int() {
        T res = 0;
        while (d < end && (*d < '0' || *d > '9')) ++d;

        // Read out digit by digit
        while (d < end && (*d >= '0' && *d <= '9')) {
            res = res*10 + (*d-'0');
            ++d;
        }
        return res;
    }

    template<typename T>
    inline
    T FileReader::read_fp() {
        T res = 0.0;
        while (d < end && !((*d >= '0' && *d <= '9') || *d == 'e' ||
                    *d == 'E' || *d == '-' || *d == '+' || *d == '.')) ++d;
        // Read the size
        bool positive = true;
        if (*d == '-') {
            positive = false;
            ++d;
        } else if (*d == '+') ++d;

        // Support a simple form of floating point integers
        // Note: this is not the most accurate or fastest strategy
        // (+-)AAA.BBB(eE)(+-)ZZ.YY
        // Read the 'A' part
        while (d < end && (*d >= '0' && *d <= '9')) {
            res = res*10. + (T)(*d-'0');
            ++d;
        }
        if (*d == '.') {
            ++d;
            T fraction = 0.;
            size_t fraction_count = 0;
            // Read the 'B' part
            while (d < end && (*d >= '0' && *d <= '9')) {
                fraction = fraction*10. + (T)(*d-'0');
                ++d;
                ++fraction_count;
            }
            res += fraction / std::pow(10., fraction_count);
        }
        if (*d == 'e' || *d == 'E') {
            ++d;
            T exp = read_fp<T>();
            res *= std::pow(10., exp);
        }

        if (!positive) res *= -1;
        return res;
    }

    inline
    bool FileReader::at_end_of_line() {
        FilePos td = d;
        while (td < end && *td != '\n') {
            if (*td != ' ' && *td != '\r')
                return false;
            ++td;
        }
        return true;
    }

    inline
    void FileReader::move_to_non_int() {
        while (d < end && (*d >= '0' && *d <= '9')) ++d;
    }

    inline
    void FileReader::move_to_non_fp() {
        while (d < end && ((*d >= '0' && *d <= '9') || *d == 'e' ||
                    *d == 'E' || *d == '-' || *d == '+' || *d == '.')) ++d;
    }

    inline
    void FileReader::move_to_fp() {
        while (d < end && !((*d >= '0' && *d <= '9') || *d == 'e' ||
                    *d == 'E' || *d == '-' || *d == '+' || *d == '.')) ++d;
    }

    inline
    void FileReader::move_to_first_int() {
        // Move through the non-ints and comments
        if (*d == '%' || *d == '#') skip_comments();
        while (d < end && (*d < '0' || *d > '9')) {
            ++d;
            if (*d == '%' || *d == '#') skip_comments();
        }
    }

    inline
    void FileReader::move_to_next_int() {
        // Move through the current int
        move_to_non_int();

        // Move through the non-ints to the next int
        move_to_first_int();
    }

    inline
    void FileReader::move_to_next_signed_int() {
        if (*d == '+' || *d == '-') ++d;
        move_to_non_int();

        // Move to the next integer or signed integral value
        if (*d == '%' || *d == '#') skip_comments();
        while (d < end && (*d < '0' || *d > '9') && *d != '+' && *d != '-') {
            ++d;
            if (*d == '%' || *d == '#') skip_comments();
        }
    }

    inline
    void FileReader::move_to_next_int_or_nl() {
        bool at_int = false;
        if (d < end && (*d >= '0' && *d <= '9')) at_int = true;
        // Move through the current int or newline
        move_to_non_int();
        if (d < end && *d == '\n') {
            if (at_int) return;     // We have now reached a newline
            ++d; // Move through a newline
        }

        if (*d == '%' || *d == '#') {
            skip_comments();
            --d;        // This will end at a newline
            return;
        }
        while (d < end && (*d < '0' || *d > '9') && *d != '\n') {
            ++d;
            if (*d == '%' || *d == '#') {
                skip_comments();
                --d;
                return;
            }
        }
    }

    inline
    void FileReader::move_to_eol() {
        while (d < end && *d != '\n') { ++d; }
    }

    inline
    bool FileReader::at_str(std::string s) {
        // Ensure the size is suitable for comparison
        if (d + s.size() >= end) return false;
        std::string d_str { d, d+s.size() };
        return s.compare(d_str) == 0;
    }

    inline
    void parallel_write(FilePos &fp, char* v, size_t v_size) {
        WFilePos wfp = (WFilePos)(fp);
        #pragma omp parallel
        {
            int num_threads = omp_get_num_threads();
            int thread_id = omp_get_thread_num();

            size_t my_data = v_size/num_threads;
            // Give the last thread the remaining data
            if (thread_id == num_threads-1)
                my_data = v_size-my_data*(num_threads-1);

            size_t start_pos = (thread_id*(v_size/num_threads));

            // Memcpy the region
            char* o_out = (char*)memcpy(wfp + start_pos,
                    v + start_pos, my_data);
            if (o_out != fp+start_pos)
                throw Error("Unable to write");
        }
        fp += v_size;
    }

    inline
    void parallel_read(FilePos &fp, char* v, size_t v_size) {
        #pragma omp parallel
        {
            int num_threads = omp_get_num_threads();
            int thread_id = omp_get_thread_num();

            size_t my_data = v_size/num_threads;
            // Give the last thread the remaining data
            if (thread_id == num_threads-1)
                my_data = v_size-my_data*(num_threads-1);

            size_t start_pos = (thread_id*(v_size/num_threads));

            // Memcpy the region
            char* o_in = (char*)memcpy(v + start_pos,
                    fp + start_pos, my_data);
            if (o_in != v+start_pos)
                throw Error("Unable to read");
        }
        fp += v_size;
    }

    namespace detail {
        template <bool wgt, class W, class O>
        struct weight_size_i_ {
            static size_t op_(O) { return 0; }
        };
        template <class W, class O>
        struct weight_size_i_<true, W, O> {
            static size_t op_(O m) { return sizeof(W)*m; }
        };
        template <bool wgt, class W, class O>
        size_t weight_size_(O m) {
            return weight_size_i_<wgt, W, O>::op_(m);
        }

        template <bool B>
        struct if_true_i_ {
            static bool op_() { return false; }
        };
        template <>
        struct if_true_i_<true> {
            static bool op_() { return true; }
        };
        template <bool B>
        bool if_true_() {
            return if_true_i_<B>::op_();
        }
    }
}
