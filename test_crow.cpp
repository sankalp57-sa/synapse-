#include "crow_all.h"
int main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });
    // Don't actually run, just compile test
    return 0;
}
