// build and run:
//
// $ g++ libwebsockets.cpp -lwebsockets -ljvar -lddpserver -o example
// $ ./example

#include "jvar.h"
#include "ddpserver.h"
#include "libwebsockets.h"

DdpServer *ddpServer;
unsigned char buf[2048];
unsigned char *buf_start;
unsigned char *buf_end;

jvar::Variant myMethod1(void *context, jvar::Variant& args)
{
    return args[0] + args[1];
}

void addHeader(lws *wsi, lws_token_indexes token, std::string s, unsigned char **pp)
{
    lws_add_http_header_by_token(wsi, token, (unsigned char *)s.c_str(), s.length(), pp, buf_end);
}
void writeToWebSocket(lws *wsi, std::string s, lws_write_protocol protocol)
{
    for (int i=0; i < s.length(); i++)
        *(buf_start + i) = s[i];

    lws_write(wsi, buf_start, s.length(), protocol);
}

int websocketCallback(lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len)
{

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED: {
            printf("connection established\n");
            writeToWebSocket(wsi, "o", LWS_WRITE_TEXT);
            break;
        }

        case LWS_CALLBACK_HTTP: {
            char *requested_uri = (char *) in;
            printf("requested URI: %s\n", requested_uri);

            unsigned char *p = buf_start;
            lws_add_http_header_status(wsi, 200, &p, buf_end);
            addHeader(wsi, WSI_TOKEN_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN, "*", &p);
            addHeader(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, "application/json", &p);
            lws_finalize_http_header(wsi, &p, buf_end);
            lws_write(wsi, buf_start, p - buf_start, LWS_WRITE_HTTP_HEADERS);

            jvar::Variant body;
            body.createObject("{ websocket: true, origins: ['*:*'], cookie_needed: false }");
            body.addProperty("entropy", "");
            body["entropy"] += rand()%10000+10000;
            body["entropy"] += rand()%10000+10000;

            writeToWebSocket(wsi, body.toJsonString(), LWS_WRITE_HTTP);
            return -1;
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            ddpServer->setContext(wsi);
            ddpServer->process((char *)in);
            break;
        }
        default:
            break;
    }

    return 0;

}

void emitCallback(void *context, const std::string &output)
{
    lws *wsi = (lws *)context;
    writeToWebSocket(wsi, output, LWS_WRITE_TEXT);
}

static struct lws_protocols protocols[] = {
    {
        "all",              /* name */
        websocketCallback,  /* callback */
        0,                  /* per_session_data_size */
        0                   /* max frame size / rx buffer */
    },
    { NULL, NULL, 0, 0 }
};

int main(int argc, char** argv)
{
    ddpServer = new DdpServer(emitCallback);
    ddpServer->registerMethod("myMethod1", myMethod1);

    buf_start = buf + LWS_SEND_BUFFER_PRE_PADDING;
    buf_end = buf_start + sizeof(buf) - LWS_SEND_BUFFER_PRE_PADDING;

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof info);
    info.port = 9002;
    info.protocols = protocols;

    struct lws_context *context = lws_create_context(&info);

    while (true)
    {
        lws_service(context, 50);
    }

}
