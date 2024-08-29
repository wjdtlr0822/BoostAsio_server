#include "ServerHeader.h"

int main() {
    cout << "Server Start......." << endl;
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 9999);
        cout << "Server Start!!!!" << endl;
        io_context.run();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
