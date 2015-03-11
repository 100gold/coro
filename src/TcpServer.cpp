
#include "TcpServer.h"
#include "ThreadPool.h"
#include "Coro.h"

using boost::system::error_code;
using boost::system::system_error;
using namespace boost::asio::ip;

TcpServer::TcpServer(const tcp::endpoint& endpoint)
	: _acceptor(ThreadPool::current()->ioService(), endpoint)
{
	boost::asio::socket_base::reuse_address option(true);
	_acceptor.set_option(option);
}

tcp::socket TcpServer::accept() {
	tcp::socket socket(ThreadPool::current()->ioService());

	Coro& coro = *Coro::current();
	error_code errorCode;

	auto callback = [&](const error_code& ec) {
		if (ec) {
			errorCode = ec;
		}
		coro.resume();
	};

	coro.yield([&]() {
		_acceptor.async_accept(socket, callback);
	});

	if (errorCode) {
		throw system_error(errorCode);
	}

	return socket;
}

void TcpServer::run(std::function<void(tcp::socket)> callback) {
	while (true) {
		auto socket = std::make_shared<tcp::socket>(accept());
		_coroPool.exec([callback, socket = std::move(socket)]() {
			callback(std::move(*socket));
		});
	}
}
