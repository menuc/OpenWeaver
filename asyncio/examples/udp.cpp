#include "marlin/asyncio/udp/UdpTransportFactory.hpp"
#include <uv.h>
#include <spdlog/spdlog.h>

using namespace marlin::core;
using namespace marlin::asyncio;

struct Delegate {
	void did_recv(UdpTransport<Delegate> &transport, Buffer &&packet) {
		SPDLOG_INFO(
			"Transport: {{Src: {}, Dst: {}}}, Did recv packet: {} bytes",
			transport.src_addr.to_string(),
			transport.dst_addr.to_string(),
			packet.size()
		);
		transport.close();
	}

	void did_send(UdpTransport<Delegate> &transport, Buffer &&packet) {
		SPDLOG_INFO(
			"Transport: {{Src: {}, Dst: {}}}, Did send packet: {} bytes",
			transport.src_addr.to_string(),
			transport.dst_addr.to_string(),
			packet.size()
		);
		transport.close();
	}

	void did_dial(UdpTransport<Delegate> &transport) {
		transport.send(
			UdpTransport<Delegate>::MessageType(10)
			.set_payload({0,0,0,0,0,0,0,0,0,0})
		);
	}

	void did_close(UdpTransport<Delegate> &, uint16_t) {}

	bool should_accept(SocketAddress const &) {
		return true;
	}

	void did_create_transport(UdpTransport<Delegate> &transport) {
		transport.setup(this);
	}
};

int main() {
	UdpTransportFactory<Delegate, Delegate> s, c;
	s.bind(SocketAddress::loopback_ipv4(8000));

	Delegate d;

	s.listen(d);

	c.bind(SocketAddress::loopback_ipv4(0));
	c.dial(SocketAddress::loopback_ipv4(8000), d);

	c.get_transport(SocketAddress::loopback_ipv4(1234));

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
