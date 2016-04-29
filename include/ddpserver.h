#ifndef DDPSERVER_H
#define DDPSERVER_H

#include "jvar.h"

class DdpServer {

private:

    typedef jvar::Variant (*method_type)(void *, jvar::Variant&);
    typedef void (*emit_callback_type)(void *, const std::string&);

    jvar::PropArray<method_type> methods;
    void *context;
    emit_callback_type onEmitCallback;

    std::string getRandomId(int len);

    jvar::Variant processPing(jvar::Variant &packet);
    jvar::Variant processConnect(jvar::Variant &packet);
    jvar::Variant processSub(jvar::Variant &packet);
    jvar::Variant processUnsub(jvar::Variant &packet);
    jvar::Variant processMethod(jvar::Variant &packet);

public:

    DdpServer(emit_callback_type emitCallback);
    void setContext(void *context);

    void process(const std::string &input);
    void emitAdd(std::string collectionName, std::string id, jvar::Variant &fields);
    void emitChange(std::string collectionName, std::string id, jvar::Variant &fields);
    void emitRemove(std::string collectionName, std::string id);

    void registerMethod(std::string name, method_type method);

};


#endif // DDPSERVER_H
