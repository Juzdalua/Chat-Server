#include "pch.h"
#include "HttpCore.h"

void handle_request(tcp::socket& socket) {
	try {
		beast::flat_buffer buffer; // ������ ���� ����

		// ��û ����
		http::request<http::string_body> req;
		http::read(socket, buffer, req); // req ��ü�� ���� ����

		// ���� �ۼ�
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.set(http::field::server, "Boost.Beast Server"); // ���� �̸�
		res.set(http::field::content_type, "text/plain"); // content_type
		res.body() = "Hello, World!"; // body
		res.prepare_payload(); // ������ ����

		// ���� ����
		http::write(socket, res);
	}
	catch (exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
}