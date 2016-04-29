// build and run:
//
// $ g++ websocketpp.cpp -lboost_system -lboost_chrono -ljvar -lddpserver -o example
// $ ./example

#include "jvar.h"
#include "ddpserver.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <boost/lexical_cast.hpp>

typedef websocketpp::server<websocketpp::config::asio> websocket_server;

websocket_server *wsServer;
DdpServer *ddpServer;

jvar::Variant myMethod1(void *context, jvar::Variant& args)
{
	return args[0] + args[1];
}

void emitCallback(void *context, const std::string &output)
{
    websocketpp::connection_hdl hdl = *(websocketpp::connection_hdl *)context;
    wsServer->send(hdl, output.c_str(), output.length(), websocketpp::frame::opcode::text);
}

void websocket_on_open(websocketpp::connection_hdl hdl)
{
    wsServer->send(hdl, "o", 1, websocketpp::frame::opcode::text);
}

void websocket_on_message(websocketpp::connection_hdl hdl, websocket_server::message_ptr msg)
{
    ddpServer->setContext(&hdl);
    ddpServer->process(msg->get_payload());
}

// sockjs emulation
void websocket_on_http(websocketpp::connection_hdl hdl) {
	websocket_server::connection_ptr con = wsServer->get_con_from_hdl(hdl);

	con->set_status(websocketpp::http::status_code::ok);
	con->append_header("access-control-allow-origin", "*");
	con->append_header("content-type", "application/json; charset=UTF-8");

	jvar::Variant body;
	body.createObject("{ websocket: true, origins: ['*:*'], cookie_needed: false }");
	body.addProperty("entropy", "");
	body["entropy"] += rand()%10000+10000;
    body["entropy"] += rand()%10000+10000;
	con->set_body(body.toJsonString().c_str());
}

int main(int argc, char** argv)
{
	ddpServer = new DdpServer(emitCallback);
	ddpServer->registerMethod("myMethod1", myMethod1);

	wsServer = new websocket_server();
    wsServer->init_asio();
    wsServer->set_http_handler(&websocket_on_http);
    wsServer->set_open_handler(&websocket_on_open);
    wsServer->set_message_handler(&websocket_on_message);
    wsServer->set_reuse_addr(true);
    wsServer->listen(9002);
    wsServer->start_accept();
    wsServer->run();
}
