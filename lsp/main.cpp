//
// hoshi-lsp — Language Server for hoshi-lang
// Entry point
//

#include "server.h"
#include <cstdio>

int main(int argc, const char **argv) {
    // if (argc > 1) {
    //     freopen(argv[1], "r", stdin);
    // }
    lsp::LspServer server;
    server.run();
    return 0;
}
