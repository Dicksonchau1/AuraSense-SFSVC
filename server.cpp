#include "engine.h"

#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXNetSystem.h>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <chrono>

int main()
{
    ix::initNetSystem();

    MultiLaneEngine engine;
    engine.start();

    const int port = 8000;
    ix::WebSocketServer server(port);

    std::mutex clientsMutex;
    std::vector<ix::WebSocket*> clients;

    std::atomic<bool> running{ true };

    server.setOnClientMessageCallback(
        [&](std::shared_ptr<ix::ConnectionState>,
            ix::WebSocket& webSocket,
            const ix::WebSocketMessagePtr& msg)
        {
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.push_back(&webSocket);
                std::cout << "Client connected\n";
            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.erase(
                    std::remove(clients.begin(), clients.end(), &webSocket),
                    clients.end());
                std::cout << "Client disconnected\n";
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                std::cout << "WebSocket error: "
                          << msg->errorInfo.reason << "\n";
            }
        });

    auto res = server.listen();
    if (!res.first)
    {
        std::cerr << "Listen error: " << res.second << std::endl;
        return 1;
    }

    server.start();

    std::cout << "Server listening on ws://127.0.0.1:8000\n";

    // ✅ Broadcast thread
    std::thread broadcaster([&]()
    {
        while (running)
        {
            Metrics m = engine.get_metrics();

            std::string json =
                "{"
                "\"hz\":" + std::to_string(m.hz) + ","
                "\"p95\":" + std::to_string(m.p95) + ","
                "\"p99\":" + std::to_string(m.p99) + ","
                "\"frames\":" + std::to_string(m.frames) + ","
                "\"cracks\":" + std::to_string(m.cracks) +
                "}";

            {
                std::lock_guard<std::mutex> lock(clientsMutex);

                for (auto* client : clients)
                {
                    if (client && client->getReadyState() == ix::ReadyState::Open)
                    {
                        client->send(json);
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    });

    // ✅ Keep main thread alive
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    running = false;
    broadcaster.join();
    engine.stop();

    return 0;
}