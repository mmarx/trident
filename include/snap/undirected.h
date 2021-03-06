#ifndef _UNDIR_H
#define _UNDIR_H

#include <snap/directed.h>
#include <snap/tunode.h>
#include <snap/tedge.h>

/*****************************
 ****** UNDIRECTED GRAPH *****
 *****************************/

class Trident_UTNGraph {
    public:
        typedef ::TUNode TNode;
        typedef ::TEdgeI TEdgeI;
        typedef Trident_UTNGraph TNet;
        typedef ::TUNode TNodeI; //Essentially the same object

    private:
        KB *kb;
        Querier *q;

        std::unique_ptr<MemoryMappedFile> mf;
        const char *rawnodes;
        int64_t nnodes;

    public:
        Trident_UTNGraph(KB *kb);

        Querier *getQuerier() {
            return q;
        }

        bool IsNode(int64_t id) {
            return id >= 0 && id < nnodes;
        }

        /// Returns the number of nodes in the graph.
        int64_t GetNodes() const {
            return nnodes;
        }

        /// Returns the number of edges in the graph.
        int64_t GetEdges() const {
            return kb->getSize();
        }

        /// Returns an iterator referring to the first node in the graph.
        Trident_UTNGraph::TNodeI BegNI() const {
            return TNodeI(rawnodes);
        }

        /// Returns an iterator referring to the past-the-end node in the graph.
        Trident_UTNGraph::TNodeI EndNI() const {
            return TNodeI(rawnodes + nnodes * 18);
        }

        Trident_UTNGraph::TEdgeI BegEI() const {
            return Trident_UTNGraph::TEdgeI(q->getPermuted(IDX_PSO, 0, -1, -1, false), q);
        }

        Trident_UTNGraph::TEdgeI EndEI() const {
            return Trident_UTNGraph::TEdgeI(NULL, NULL);
        }

        /// Returns an iterator referring to the node of ID NId in the graph.
        Trident_UTNGraph::TNodeI GetNI(const int64_t& NId) const {
            return TNodeI(rawnodes + 18 * NId);
        }

        /// Gets a vector IDs of all nodes in the graph.
        void GetNIdV(std::vector<int64_t>& NIdV) const {
            NIdV.resize(GetNodes());
            for (int64_t i = 0; i < nnodes; ++i) {
                const int64_t v = (*(int64_t*)(rawnodes + 18 * i)) & 0XFFFFFFFFFFl;
                NIdV[i] = v;
            }
        }

        bool HasFlag(const TGraphFlag& Flag) const {
            switch (Flag) {
                case gfDirected:
                    return false;
                case gfMultiGraph:
                    return false;
                case gfBipart:
                    return false;
                default:
                    return false;
            }
        }

        ~Trident_UTNGraph() {
            delete q;
        }


    public:
        TCRef CRef;
};

typedef TPt<Trident_UTNGraph> PTrident_UTNGraph;

#endif
