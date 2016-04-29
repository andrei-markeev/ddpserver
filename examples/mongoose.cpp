// build and run:
//
// $ g++ mongoose.cpp -lmongoose -ljvar -lddpserver -o example
// $ ./example

// NOTE: We need to add this dependency still
#include "mongoose.h"
#include <ddpserver.h>
#include <jvar/jvar.h>

DdpServer *ddpServer;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {

  switch(ev){

      case MG_EV_HTTP_REQUEST:
      {
          mg_printf(nc, "HTTP/1.1 200\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-Type: application/json; charset=UTF-8\r\n\r\n");

          mg_printf(nc, "{ \"websocket\": true, \"origins\": [\"*:*\"], \"cookie_needed\": false, \"entropy\": \"%d%d\" }",
                        rand()%10000+10000, rand()%10000+10000);

          nc->flags |= MG_F_SEND_AND_CLOSE;
          break;
      }
      case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
      {
          mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, "o", 1);
          break;
      }
      case MG_EV_WEBSOCKET_FRAME:
      {
          struct websocket_message *wm = (struct websocket_message *) p;
          std::string s((char *)wm->data, wm->size);
          ddpServer->setContext(nc);
          ddpServer->process(s);
      }

  }

}

jvar::Variant myMethod1(void *context, jvar::Variant& args)
{
    return args[0] + args[1];
}

void emitCallback(void *context, const std::string &output)
{
    struct mg_connection *nc = (struct mg_connection *)context;
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, output.c_str(), (size_t)output.length());
}


int main(void) {

    ddpServer = new DdpServer(emitCallback);
    ddpServer->registerMethod("myMethod1", myMethod1);

    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, "9002", ev_handler);
    mg_set_protocol_http_websocket(nc);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}
