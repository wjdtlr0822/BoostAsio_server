#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
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
    ChatRoom::clients_.erase(socket);
}

void ChatRoom::broadcast(const string& message) {
    for (auto& client : clients_) {
        boost::asio::async_write(*client,
            boost::asio::buffer(message),
            [this](boost::system::error_code, std::size_t) {});
    }
}

class Session : public enable_shared_from_this<Session> {
public:
    // io_context를 받아서 strand를 초기화하도록 수정
    Session(boost::asio::io_context& io_context, shared_ptr<tcp::socket> socket, shared_ptr<ChatRoom> room)
        : socket_(socket), room_(room), strand_(boost::asio::make_strand(io_context)) {}

    void start();
    void do_read();

private:
    shared_ptr<tcp::socket> socket_;
    shared_ptr<ChatRoom> room_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    string read_buffer_;
};

void Session::start() {
    room_->join(socket_);
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    boost::asio::async_read_until(*socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        boost::asio::bind_executor(strand_, [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                std::string message = read_buffer_;
                read_buffer_.clear();
                room_->broadcast(message);
                do_read();
            }
            else {
                room_->leave(socket_);
            }
            }));
}

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept();

    // io_context 멤버 추가
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

void Server::do_accept() {
    auto socket = make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            auto room = make_shared<ChatRoom>();
            // io_context를 Session에 전달
            make_shared<Session>(io_context_, socket, room)->start();
            cout << "Client Access " << endl;
        }
    do_accept();
        });
}
