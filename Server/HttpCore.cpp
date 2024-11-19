#include "pch.h"
#include "HttpCore.h"

void handle_request(tcp::socket& socket) {
	try {
		beast::flat_buffer buffer;

		// 요청 수신
		http::request<http::string_body> req;
		http::read(socket, buffer, req);

		// 응답 작성
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.set(http::field::server, "Boost.Beast Server");
		res.set(http::field::content_type, "text/plain");
		res.body() = "Hello, World!";
		res.prepare_payload();

		// 응답 전송
		http::write(socket, res);
	}
	catch (exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
}