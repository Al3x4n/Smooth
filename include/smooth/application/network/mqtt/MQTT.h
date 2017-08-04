//
// Created by permal on 7/20/17.
//

#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <deque>
#include <unordered_map>
#include <smooth/core/Task.h>
#include <smooth/core/ipc/Mutex.h>
#include <smooth/core/network/IPv4.h>
#include <smooth/core/network/DataAvailableEvent.h>
#include <smooth/core/network/ConnectionStatusEvent.h>
#include <smooth/core/network/TransmitBufferEmptyEvent.h>
#include <smooth/core/network/Socket.h>
#include <smooth/core/network/SSLSocket.h>
#include <smooth/core/network/PacketSendBuffer.h>
#include <smooth/core/network/PacketReceiveBuffer.h>
#include <smooth/core/ipc/TaskEventQueue.h>
#include <smooth/application/network/mqtt/packet/MQTTPacket.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/application/network/mqtt/state/MqttFSM.h>
#include <smooth/application/network/mqtt/state/MQTTBaseState.h>
#include <smooth/application/network/mqtt/IMqtt.h>

namespace smooth
{
    namespace application
    {
        namespace network
        {
            namespace mqtt
            {
                // The MQTT class handles everything required for a connection to a single MQTT broker,
                // such as connecting, subscribing and publish topics.
                class MQTT
                        : public smooth::core::Task,
                          public IMqtt,
                          public core::ipc::IEventListener<core::network::TransmitBufferEmptyEvent>,
                          public core::ipc::IEventListener<core::network::DataAvailableEvent<packet::MQTTPacket>>,
                          public core::ipc::IEventListener<core::network::ConnectionStatusEvent>,
                          public core::ipc::IEventListener<core::timer::TimerExpiredEvent>
                {
                    public:
                        MQTT(const std::string& mqtt_client_id, std::chrono::seconds keep_alive, uint32_t stack_depth, UBaseType_t priority);

                        void
                        connect_to(std::shared_ptr<smooth::core::network::InetAddress> address, bool auto_reconnect,
                                   bool use_ssl);
                        void disconnect();
                        void shutdown_connection() override;

                        void init() override;

                        void message(const core::network::TransmitBufferEmptyEvent& msg);
                        void message(const core::network::ConnectionStatusEvent& msg);
                        void message(const packet::MQTTPacket& msg);
                        void message(const core::network::DataAvailableEvent<packet::MQTTPacket>& msg);
                        void message(const core::timer::TimerExpiredEvent& msg);

                        const std::string& get_client_id() const override;
                        const std::chrono::seconds get_keep_alive() const override;
                        void start_reconnect() override;
                        void reconnect() override;
                        bool get_auto_reconnect() const override;
                        void set_keep_alive_timer(std::chrono::seconds interval) override;

                    protected:
                        void tick() override;

                    private:

                        void send_packet(packet::MQTTPacket& packet, std::chrono::milliseconds timeout) override;

                        core::network::PacketSendBuffer<packet::MQTTPacket, 5> tx_buffer;
                        core::network::PacketReceiveBuffer<packet::MQTTPacket, 5> rx_buffer;
                        core::ipc::TaskEventQueue<core::network::TransmitBufferEmptyEvent> tx_empty;
                        core::ipc::TaskEventQueue<core::network::DataAvailableEvent<packet::MQTTPacket>> data_available;
                        core::ipc::TaskEventQueue<smooth::core::network::ConnectionStatusEvent> connection_status;
                        core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent> timer_events;
                        smooth::core::ipc::Mutex guard;
                        std::string client_id;
                        std::chrono::seconds keep_alive;
                        std::unique_ptr<smooth::core::network::ISocket> mqtt_socket;
                        core::timer::Timer receive_timer;
                        core::timer::Timer reconnect_timer;
                        core::timer::Timer keep_alive_timer;
                        smooth::application::network::mqtt::state::MqttFSM<state::MQTTBaseState> fsm;
                        bool auto_reconnect = false;
                };
            }
        }
    }
}