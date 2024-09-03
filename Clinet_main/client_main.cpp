#include "ClientHeader.h"

string makemessage(string message,uint16_t proto) {
    string full_message;
    int16_t length = message.length() + 4;
    full_message.resize(4);

    *reinterpret_cast<uint16_t*>(&full_message[0]) = htons(length);
    *reinterpret_cast<uint16_t*>(&full_message[4]) = htons(proto);
    full_message.append(message);

    return full_message;
}

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "9999");
        Client client(io_context, endpoints);

        cout << "Client Connect Success" << endl;
        thread t([&io_context]() { io_context.run(); });

        string orimessage;
        while (getline(cin, orimessage)) {
            string message = makemessage(orimessage, CREATE_ROOM);
            client.write(message);
        }

        client.close();
        t.join();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
