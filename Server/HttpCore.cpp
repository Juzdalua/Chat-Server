#include "pch.h"
#include "HttpCore.h"

void handle_request(tcp::socket& socket) {
	try {
		beast::flat_buffer buffer; // 데이터 수신 버퍼

		// 요청 수신
		http::request<http::string_body> req;
		http::read(socket, buffer, req); // req 객체에 정보 저장

		// 응답 작성
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.set(http::field::server, "Boost.Beast Server"); // 서버 이름
		res.set(http::field::content_type, "text/plain"); // content_type
		res.body() = "Hello, World!"; // body
		res.prepare_payload(); // 데이터 설정

		// 응답 전송
		http::write(socket, res);
	}
	catch (exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
}