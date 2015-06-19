
#include "coro/IoService.h"
#include "coro/CoroPool.h"
#include "coro/TcpServer.h"
#include "coro/TcpSocket.h"
#include "coro/StreamIterator.h"
#include "coro/WsProtocol.h"
#include <iostream>

using namespace std;
using namespace boost::asio::ip;

class WsEchoSession {
public:
	WsEchoSession(TcpSocket socket): _socket(move(socket))
	{
		cout << "WsEchoSession" << endl;
	}
	~WsEchoSession() {
		cout << "~WsEchoSession" << endl;
	}

	void doHandshake() {
		StreamIterator<TcpSocket, Buffer> it(_socket, *_inputBuffer), end;
		auto outputBuffer = MallocBuffer();
		_inputBuffer->popFront(_wsProtocol.doHandshake(it, end, *outputBuffer));
		_socket.write(*outputBuffer);
	}

	void printMessage(const WsMessage& message) {
		if (message.opCode() == WsMessage::OpCode::Text) {
			copy(message.payloadBegin(), message.payloadEnd(),
				ostream_iterator<char>(cout));
			cout << endl;
		}
	}

	void run() {
		try {
			doHandshake();

			while (true) {
				StreamIterator<TcpSocket, Buffer> it(_socket, *_inputBuffer), end;
				WsMessage message = _wsProtocol.readMessage(it, end);

				if (message.opCode() == WsMessage::OpCode::Close) {
					auto outputBuffer = MallocBuffer();
					_wsProtocol.writeMessage(WsMessage::OpCode::Close, *outputBuffer);
					_socket.write(*outputBuffer);
					return;
				}

				printMessage(message);

				auto outputBuffer = MallocBuffer();
				// копируем payload
				outputBuffer->assign(message.payloadBegin(), message.payloadEnd());
				// запаковываем в websockets
				_wsProtocol.writeMessage(message.opCode(), *outputBuffer);
				// отправляем
				_socket.write(*outputBuffer);

				_inputBuffer->popFront(message.end());
			}
		}
		catch (const exception& error) {
			cout << error.what() << endl;
		}
	}

private:
	TcpSocket _socket;
	WsProtocol _wsProtocol;
	BufferUniquePtr _inputBuffer = MallocBuffer();
};

void StartAccept() {
	auto endpoint = tcp::endpoint(address_v4::from_string("127.0.0.1"), 44442);
	TcpServer server(endpoint);
	server.run([](TcpSocket socket) {
		WsEchoSession session(std::move(socket));
		session.run();
	});
}

int main() {
	Coro coro(StartAccept);

	IoService ioService;
	ioService.post([&] {
		coro.resume();
	});
	ioService.run();

	return 0;
}
