#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <deque>
#include <cstdint>
#include "protocolHeader.h"


using boost::asio::ip::tcp;
using namespace std;

class Client {
public:
    Client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
        : socket_(io_context), strand_(boost::asio::make_strand(io_context)) {
        do_connect(endpoints);
    }

    void write(const string& message);
    void close();

private:
    void do_connect(const tcp::resolver::results_type& endpoints);
    void do_read();
    void do_write();

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    string read_buffer_;
    deque<string> write_msgs_;
};

void Client::write(const string& message) {
    boost::asio::post(strand_, [this, message]() {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(message + "\n");
        if (!write_in_progress) {
            do_write();
        }
        });
}

void Client::close() {
    boost::asio::post(strand_, [this]() { socket_.close(); });
}


void Client::do_connect(const tcp::resolver::results_type& endpoints) {
    boost::asio::async_connect(socket_, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint) {
            if (!ec) {
                do_read();
            }
        });
}

void Client::do_read() {
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        boost::asio::bind_executor(strand_,
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    cout << "Received: " << read_buffer_.substr(0, length);
                    read_buffer_.erase(0, length);
                    do_read();
                }
                else {
                    socket_.close();
                }
            }));
}

void Client::do_write() {
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_msgs_.front()),
        boost::asio::bind_executor(strand_,
            [this](boost::system::error_code ec, size_t) {
                if (!ec) {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty()) {
                        do_write();
                    }
                }
                else {
                    socket_.close();
                }
            }));
}