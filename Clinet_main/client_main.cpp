#include "ClientHeader.h"

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "9999");
        Client client(io_context, endpoints);

        cout << "Client Connect Success" << endl;
        thread t([&io_context]() { io_context.run(); });

        string line;
        while (getline(cin, line)) {
            client.write(line);
        }

        client.close();
        t.join();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
