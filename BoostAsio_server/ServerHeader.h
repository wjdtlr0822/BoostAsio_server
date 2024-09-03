#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "protocolHeader.h"

using boost::asio::ip::tcp;
using namespace std;

class ChatRoom {
public:
    void join(shared_ptr<tcp::socket> socket);
    void leave(shared_ptr<tcp::socket> socket);
    void broadcast(const string& message);

private:
    set<shared_ptr<tcp::socket>> clients_;
};

void ChatRoom::join(shared_ptr<tcp::socket> socket) {
    clients_.insert(socket);
}

void ChatRoom::leave(shared_ptr<tcp::socket> socket) {
    clients_.erase(socket);
}

void ChatRoom::broadcast(const string& message) {
    for (auto& client : clients_) {
        boost::asio::async_write(*client,
            boost::asio::buffer(message),
            [this](boost::system::error_code, std::size_t) {});
    }
}


/*==============================================================================================================*/
class Session : public enable_shared_from_this<Session> {
public:
    Session(boost::asio::io_context& io_context, shared_ptr<tcp::socket> socket, shared_ptr<unordered_map<string, shared_ptr<ChatRoom>>> rooms)
        : socket_(socket), rooms_(rooms), strand_(boost::asio::make_strand(io_context)) {}

    void start();
    void do_read_header();
    void do_read_body(uint16_t length, uint16_t code);

private:
    shared_ptr<tcp::socket> socket_;
    shared_ptr<unordered_map<string, shared_ptr<ChatRoom>>> rooms_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    string read_buffer_;
    shared_ptr<ChatRoom> current_room_;

    void handle_message(uint16_t code, const std::string& message);
};

void Session::start() {
    do_read_header();
}

void Session::do_read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buffer_, 4),
        boost::asio::bind_executor(strand_,
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    
                    uint16_t length, code;
                    std::memcpy(&length, read_buffer_.data(), sizeof(length));
                    std::memcpy(&code, read_buffer_.data() + 2, sizeof(code));
                    length = ntohs(length);
                    code = ntohs(code);

                    read_buffer_.clear();
                    do_read_body(length - 4, code);
                }
                else {
                    // Handle error
                    std::cerr << "Error reading header: " << ec.message() << std::endl;
                }
            }));
}

void Session::do_read_body(uint16_t length, uint16_t code) {
    read_buffer_.resize(length);
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buffer_, length),
        boost::asio::bind_executor(strand_,
            [this, self, code](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string message(read_buffer_.data(), read_buffer_.size());
                    std::cout << "Received message: " << message << std::endl;
                    handle_message(code, message);
                    do_read_header(); 
                }
                else {
                    std::cerr << "Error reading body: " << ec.message() << std::endl;
                    if (current_room_) {
                        current_room_->leave(socket_);
                    }
                }
            }));
}

void Session::handle_message(uint16_t code, const std::string& message){
    switch (code) {
    case CREATE_ROOM:
        break;
    case DELETE_ROOM:
        break;
    case SEND_MESSAGE:
        if (current_room_) {
            current_room_->broadcast(message);
        }
        break;
    default:
        break;
    }
}
/*==============================================================================================================*/
class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), rooms_(make_shared<unordered_map<string, shared_ptr<ChatRoom>>>()) {
        do_accept();
    }

private:
    void do_accept();

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    shared_ptr<unordered_map<string, shared_ptr<ChatRoom>>> rooms_;
};

void Server::do_accept() {
    auto socket = make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            make_shared<Session>(io_context_, socket, rooms_)->start();
            cout << "Client connected" << endl;
        }
    do_accept();
        });
}
