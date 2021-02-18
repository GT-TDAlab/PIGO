/**
 * PIGO: a parallel graph and matrix I/O and preprocessing library
 * Copyright (c) 2021 GT-TDALab
 */

namespace pigo {
    template<class V, class O, class S>
    V& EdgeItT<V,O,S>::operator*() { return get_value_<S, V&>(s, pos); }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage>
    edge_ctr_t BaseGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage>::neighbor_start(vertex_t v) {
        return get_value_<edge_ctr_storage, edge_ctr_t>(this->offsets_, v);
    }

    template<class vertex_t, class edge_ctr_t, class edge_storage, class edge_ctr_storage>
    edge_ctr_t BaseGraph<vertex_t, edge_ctr_t, edge_storage, edge_ctr_storage>::neighbor_end(vertex_t v) {
        return get_value_<edge_ctr_storage, edge_ctr_t>(this->offsets_, v+1);
    }
}
