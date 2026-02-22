#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>
#include <string>

namespace ws {

namespace beast     = boost::beast;
namespace http      = beast::http;
namespace websocket = beast::websocket;
namespace net       = boost::asio;
using tcp           = boost::asio::ip::tcp;

class WsServer;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, WsServer* owner)
        : ws_(std::move(socket)), owner_(owner) {}

    void start();

    void send_text(const std::string& msg);
    void send_binary(const void* data, std::size_t size);

private:
    websocket::stream<tcp::socket> ws_;
    WsServer* owner_;
    std::mutex send_mutex_;

    void do_read();
};

class WsServer {
public:
    explicit WsServer(unsigned short port)
        : ioc_(1),
          acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)),
          running_(false) {}

    ~WsServer() { stop(); }

    void start() {
        if (running_) return;
        running_ = true;
        do_accept();
        thread_ = std::thread([this]{ ioc_.run(); });
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        beast::error_code ec;
        acceptor_.close(ec);
        ioc_.stop();
        if (thread_.joinable()) thread_.join();
    }

    void broadcast_text(const std::string& msg) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (auto sp = it->lock()) {
                sp->send_text(msg);
                ++it;
            } else {
                it = clients_.erase(it);
            }
        }
    }

    void broadcast_binary(const void* data, std::size_t size) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (auto sp = it->lock()) {
                sp->send_binary(data, size);
                ++it;
            } else {
                it = clients_.erase(it);
            }
        }
    }

private:
    friend class Session;

    net::io_context ioc_;
    tcp::acceptor   acceptor_;
    std::thread     thread_;
    std::atomic<bool> running_;

    std::mutex                             clients_mutex_;
    std::vector<std::weak_ptr<Session>>    clients_;

    void do_accept() {
        acceptor_.async_accept(
            net::make_strand(ioc_),
            [this](beast::error_code ec, tcp::socket socket) {
                if (!running_) return;
                if (!ec) {
                    auto s = std::make_shared<Session>(std::move(socket), this);
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex_);
                        clients_.push_back(s);
                    }
                    s->start();
                }
                do_accept();
            });
    }
};

inline void Session::start() {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http::field::server, std::string("AuraSense-RT-Engine"));
        }));
    ws_.async_accept(
        [self = shared_from_this()](beast::error_code ec) {
            if (!ec) self->do_read();
        });
}

inline void Session::do_read() {
    auto buffer = std::make_shared<beast::flat_buffer>();
    ws_.async_read(
        *buffer,
        [self = shared_from_this(), buffer](beast::error_code ec, std::size_t) {
            if (!ec) self->do_read();   // ignore incoming messages
        });
}

inline void Session::send_text(const std::string& msg) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    beast::error_code ec;
    ws_.text(true);
    ws_.write(net::buffer(msg), ec);
}

inline void Session::send_binary(const void* data, std::size_t size) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    beast::error_code ec;
    ws_.binary(true);
    ws_.write(net::buffer(data, size), ec);
}

} // namespace ws