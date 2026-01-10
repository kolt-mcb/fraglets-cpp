// Minimal graphviz stubs for compilation without graphviz library
#ifndef GRAPHVIZ_STUB_H
#define GRAPHVIZ_STUB_H

// Define minimal types needed for compilation
typedef struct Agraph_s Agraph_t;
typedef struct Agnode_s Agnode_t;
typedef struct Agedge_s Agedge_t;
typedef struct GVC_s GVC_t;

#define TRUE 1
#define FALSE 0
#define Agdirected 1
#define Agundirected 0

// Stub functions (will not be called in quiet mode)
inline Agraph_t* agopen(const char*, int, void*) { return nullptr; }
inline Agnode_t* agnode(Agraph_t*, char*, int) { return nullptr; }
inline Agedge_t* agedge(Agraph_t*, Agnode_t*, Agnode_t*, const char*, int) { return nullptr; }
inline Agraph_t* agsubg(Agraph_t*, char*, int) { return nullptr; }
inline char* agsafeset(void*, const char*, const char*, const char*) { return nullptr; }
inline int agclose(Agraph_t*) { return 0; }
inline GVC_t* gvContext() { return nullptr; }
inline int gvLayout(GVC_t*, Agraph_t*, const char*) { return 0; }
inline int gvRenderFilename(GVC_t*, Agraph_t*, const char*, const char*) { return 0; }
inline int gvFreeLayout(GVC_t*, Agraph_t*) { return 0; }
inline int gvFreeContext(GVC_t*) { return 0; }

#endif // GRAPHVIZ_STUB_H
