#pragma once

#include <mc_cpp/common/macro_magic.hpp>
#include <protocolCraft/AllPackets.hpp>
#include <string>

namespace mc {
    namespace pc = ProtocolCraft;

    static constexpr int PROTOCOL = PROTOCOL_VERSION;
    static const std::string MINECRAFT_VERSION = BOTCRAFT_GAME_VERSION;

    template<typename Packet, typename TuplePackets>
    constexpr int32_t PACKET = mc::get_tuple_index<Packet, TuplePackets>;

    namespace side {
        struct C2S {
            static constexpr std::string_view asString = "C2S";
            static constexpr std::string_view boundTo = "server";
            static constexpr std::string_view source = "in";
        };

        struct S2C {
            static constexpr std::string_view asString = "S2C";
            static constexpr std::string_view boundTo = "client";
            static constexpr std::string_view source = "out";
        };
    }

    namespace state {
        struct Handshake {
            static constexpr auto state = 0;
        };

        struct Status {
            static constexpr auto state = 1;
        };

        struct Login {
            static constexpr auto state = 2;
        };

        struct Play {
            static constexpr auto state = 3;
        };

        struct Config {
            static constexpr auto state = 4;
        };

        enum class StateType {
            HANDSHAKE = Handshake::state,
            STATUS = Status::state,
            LOGIN = Login::state,
            PLAY = Play::state,
            CONFIG = Config::state,
        };

        template<typename Type>
        struct ConnectionState {
            static_assert(
                std::is_same_v<Type, Handshake>
                || std::is_same_v<Type, Status>
                || std::is_same_v<Type, Login>
                || std::is_same_v<Type, Play>
                || std::is_same_v<Type, Config>);

            static constexpr auto state = Type::state;
        };

        template<>
        struct ConnectionState<Handshake> {
            template<typename Packet>
            static constexpr auto c2s = PACKET<Packet, pc::AllServerboundHandshakingPackets>;

            template<typename Packet>
            static constexpr auto s2c = PACKET<Packet, std::tuple<> >;
        };

        template<>
        struct ConnectionState<Status> {
            template<typename Packet>
            static constexpr auto c2s = PACKET<Packet, pc::AllServerboundStatusPackets>;

            template<typename Packet>
            static constexpr auto s2c = PACKET<Packet, pc::AllClientboundStatusPackets>;
        };

        template<>
        struct ConnectionState<Login> {
            template<typename Packet>
            static constexpr auto c2s = PACKET<Packet, pc::AllServerboundLoginPackets>;

            template<typename Packet>
            static constexpr auto s2c = PACKET<Packet, pc::AllClientboundLoginPackets>;
        };

        template<>
        struct ConnectionState<Play> {
            template<typename Packet>
            static constexpr auto c2s = PACKET<Packet, pc::AllServerboundPlayPackets>;

            template<typename Packet>
            static constexpr auto s2c = PACKET<Packet, pc::AllClientboundPlayPackets>;
        };

        template<>
        struct ConnectionState<Config> {
            template<typename Packet>
            static constexpr auto c2s = PACKET<Packet, pc::AllServerboundConfigurationPackets>;

            template<typename Packet>
            static constexpr auto s2c = PACKET<Packet, pc::AllClientboundConfigurationPackets>;
        };
    }

    template<typename Side>
    struct PacketType {
        static_assert(std::is_same_v<Side, side::C2S> || std::is_same_v<Side, side::S2C>);
    };

    template<>
    struct PacketType<side::C2S> {
        template<typename State, typename Packet>
        static constexpr auto get = state::ConnectionState<State>::template c2s<Packet>;
    };

    template<>
    struct PacketType<side::S2C> {
        template<typename State, typename Packet>
        static constexpr auto get = state::ConnectionState<State>::template s2c<Packet>;
    };
}
