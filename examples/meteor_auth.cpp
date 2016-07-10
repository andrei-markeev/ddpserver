// build and run:
//
// $ g++ meteor_auth.cpp -lboost_system -lboost_chrono -ljvar -lddpserver -o example
// $ ./example

#include <jvar/jvar.h>
#include <ddpserver.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <boost/lexical_cast.hpp>

typedef websocketpp::server<websocketpp::config::asio> websocket_server;

websocket_server *wsServer;
DdpServer *ddpServer;

// perform login
jvar::Variant login(void *context, jvar::Variant& args)
{
    std::string userId = args[0];
    std::string loginToken = args[1];

    char buf[128];
    boost::system::error_code ec;
    boost::asio::io_service svc;
    boost::asio::ip::tcp::socket s(svc);
    boost::asio::ip::address localhost = boost::asio::ip::address::from_string("127.0.0.1");

    // connect to the Meteor server
    s.connect(boost::asio::ip::tcp::endpoint(localhost, 3000));

    // send request
    std::string request("GET /checkLoginToken/" + loginToken + "?userId=" + userId + "\r\nConnection: close\r\n\r\n");
    s.send(boost::asio::buffer(request));

    // TODO: rewrite to use nonblocking socket reading
    std::string response;

    size_t bytes_transferred = 0;
    do
    {
        bytes_transferred += s.receive(boost::asio::buffer(buf, 128), 0, ec);
        response.append(buf, buf + bytes_transferred);
    } while (!ec);

    if (response.find("\r\n\r\n") == std::string::npos)
        return "ACCESS_DENIED"; // unexpected response
    response = response.substr(response.find("\r\n\r\n")+4);
    response = response.substr(response.find("\r\n")+2);
    response = response.substr(0,response.find("\r\n"));

    s.close();

    if (response == "200")
        return "OK";
    else
        return "ACCESS_DENIED";
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
	ddpServer->registerMethod("login", login);

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
