#include "ddpserver.h"

DdpServer::DdpServer(emit_callback_type emitCallback)
{
    context = NULL;
    onEmitCallback = emitCallback;
}

void DdpServer::registerMethod(std::string name, method_type method)
{
    *methods.add(name.c_str()) = method;
}

void DdpServer::setContext(void *ctx)
{
    context = ctx;
}

void DdpServer::process(std::string input)
{
    jvar::Variant inputData;
    if (!inputData.parseJson(input.c_str()))
        throw;

    jvar::Variant responseArray;
    responseArray.createArray();
    for(int i = 0; i < inputData.length();i++)
    {
        jvar::Variant packet;
        packet.parseJson(inputData[i].toString().c_str());

        jvar::Variant response;
        response = processPing(packet);
        response = response.isEmpty() ? processConnect(packet) : response;
        response = response.isEmpty() ? processSub(packet) : response;
        response = response.isEmpty() ? processUnsub(packet) : response;
        response = response.isEmpty() ? processMethod(packet) : response;

        if (response.isEmpty())
        {
            // unknown
        }
        else if (response.isArray())
        {
            for (int j = 0; j < response.length(); j++)
                responseArray.push(response[j].toString());
        }
        else
        {
            responseArray.push(response.toString());
        }

    }

    if (responseArray.length() == 0)
        return;

    onEmitCallback(context, "a" + responseArray.toJsonString());
}

jvar::Variant DdpServer::processPing(jvar::Variant packet)
{
    jvar::Variant response;
    if (packet["msg"] != "ping")
        return response;

    response.createObject();
    response.addProperty("msg", "pong");
    if (!packet["id"].empty())
        response.addProperty("id", packet["id"]);

    return response;
}

jvar::Variant DdpServer::processConnect(jvar::Variant packet)
{
    jvar::Variant response;

    if (packet["msg"] != "connect")
        return response;

    response.createObject();
    response.addProperty("msg", "connected");
    if (!packet.hasProperty("session"))
        response.addProperty("session", getRandomId(10));
    else
        response.addProperty("session", packet["session"]);

    return response;
}

jvar::Variant DdpServer::processSub(jvar::Variant packet)
{
    jvar::Variant response;

    if (packet["msg"] != "sub")
        return response;

    response.createObject();

    jvar::Variant subs;
    subs.createArray();
    subs.push(packet["id"]);

    response.createObject();
    response.addProperty("msg", "ready");
    response.addProperty("subs", subs);

    return response;

}

jvar::Variant DdpServer::processUnsub(jvar::Variant packet)
{
    jvar::Variant response;

    if (packet["msg"] != "unsub")
        return response;

    // TODO: perform callback when unsubscribing?

    return response;

}


jvar::Variant DdpServer::processMethod(jvar::Variant packet)
{
    jvar::Variant response;

    if (packet["msg"] != "method")
        return response;

    jvar::Variant error;
    jvar::Variant returnValue;
    std::string methodName = packet["method"].toString();
    try
    {
        method_type *m = methods.get(methodName.c_str());
        if (!m)
        {
            error.createObject();
            error.addProperty("error", "500");
            error.addProperty("reason", "Method not found");
        }
        else
        {
            returnValue = (*m)(context, packet["params"]);
        }
    }
    catch (std::exception& ex)
    {
        error.createObject();
        error.addProperty("error", "500");
        error.addProperty("reason", ex.what());
    }

    response.createArray();

    jvar::Variant result;
    result.createObject();
    result.addProperty("msg", "result");
    result.addProperty("id", packet["id"]);
    result.addProperty("result", returnValue);
    if (!error.isEmpty())
        result.addProperty("error", error);

    jvar::Variant updatedIds;
    updatedIds.createArray();
    updatedIds.push(packet["id"]);

    jvar::Variant updated;
    updated.createObject();
    updated.addProperty("msg", "updated");
    updated.addProperty("methods", updatedIds);

    response.push(result);
    response.push(updated);
    return response;

}

std::string DdpServer::getRandomId(int len)
{
    std::string s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return s;
}

void DdpServer::emitAdd(std::string collectionName, std::string id, jvar::Variant fields)
{
    jvar::Variant data;
    data.createObject();
    data.addProperty("msg", "added");
    data.addProperty("collection", collectionName);
    data.addProperty("id", id);
    data.addProperty("fields", fields);

    jvar::Variant responseArray;
    responseArray.createArray();
    responseArray.push(data.toJsonString());
    onEmitCallback(context, "a" + responseArray.toJsonString());
}

void DdpServer::emitChange(std::string collectionName, std::string id, jvar::Variant fields)
{
    jvar::Variant data;
    data.createObject();
    data.addProperty("msg", "changed");
    data.addProperty("collection", collectionName);
    data.addProperty("id", id);
    data.addProperty("fields", fields);

    jvar::Variant responseArray;
    responseArray.createArray();
    responseArray.push(data.toJsonString());
    onEmitCallback(context, "a" + responseArray.toJsonString());
}

void DdpServer::emitRemove(std::string collectionName, std::string id)
{
    jvar::Variant data;
    data.createObject();
    data.addProperty("msg", "removed");
    data.addProperty("collection", collectionName);
    data.addProperty("id", id);

    jvar::Variant responseArray;
    responseArray.createArray();
    responseArray.push(data.toJsonString());
    onEmitCallback(context, "a" + responseArray.toJsonString());
}
