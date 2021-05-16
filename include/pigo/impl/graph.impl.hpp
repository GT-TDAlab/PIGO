/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 */

namespace pigo {
    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    DiGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::DiGraph(std::string fn) :
        DiGraph(fn, AUTO) { }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    DiGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::DiGraph(std::string fn, FileType ft) {
        ROFile f {fn};
        read_(f, ft);
    }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    DiGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::DiGraph(File& f, FileType ft) {
        read_(f, ft);
    }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    void DiGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::read_(File& f, FileType ft) {
        FileType ft_used = ft;
        // If the file type is AUTO, then try to detect it
        if (ft_used == AUTO) {
            ft_used = f.guess_file_type();
        }
        if (ft_used == PIGO_DIGRAPH_BIN) {
            // First load the in, then the out
            // Read out the header
            f.read(digraph_file_header);
            in_ = BaseGraph<
                        vertex_t,
                        edge_ctr_t,
                        edge_storage,
                        edge_ctr_storage,
                        weighted,
                        Weight,
                        WeightStorage
                    > { f, AUTO };
            out_ = BaseGraph<
                        vertex_t,
                        edge_ctr_t,
                        edge_storage,
                        edge_ctr_storage,
                        weighted,
                        Weight,
                        WeightStorage
                    > { f, AUTO };
        } else {
            // Build a COO, then load ourselves from it
            COO<
                    vertex_t, edge_ctr_t, edge_storage,
                    false, false, false, false,
                    weighted, Weight, WeightStorage
                > coo { f, ft };
            from_coo_(coo);
            coo.free();
        }
    }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    void DiGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::save(std::string fn) {
        // Find the total size to save
        size_t out_size = 0;

        std::string dfh { digraph_file_header };
        out_size += dfh.size();

        out_size += in_.save_size();
        out_size += out_.save_size();

        // Now, create the file and output everything
        WFile w {fn, out_size};

        w.write(dfh);

        in_.save(w);
        out_.save(w);
    }

    template<class V, class O, class S>
    V& EdgeItT<V,O,S>::operator*() { return detail::get_value_<S, V&>(s, pos); }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    edge_ctr_t BaseGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::neighbor_start(vertex_t v) {
        return detail::get_value_<edge_ctr_storage, edge_ctr_t>(this->offsets_, v);
    }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage, bool weighted, class Weight, class WeightStorage>
    edge_ctr_t BaseGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage, weighted, Weight, WeightStorage>::neighbor_end(vertex_t v) {
        return detail::get_value_<edge_ctr_storage, edge_ctr_t>(this->offsets_, v+1);
    }
}
