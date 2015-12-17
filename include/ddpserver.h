#ifndef DDPSERVER_H
#define DDPSERVER_H

#include <jvar/jvar.h>

class DdpServer {

private:

    jvar::Variant methods;
    jvar::Variant env;
    void (*onEmitCallback)(jvar::Variant&, std::string);

    std::string getRandomId(int len);

    jvar::Variant processPing(jvar::Variant packet);
    jvar::Variant processConnect(jvar::Variant packet);
    jvar::Variant processSub(jvar::Variant packet);
    jvar::Variant processUnsub(jvar::Variant packet);
    jvar::Variant processMethod(jvar::Variant packet);

public:

    DdpServer(void (*onEmit)(jvar::Variant&, std::string));
    void setEnv(std::string varName, jvar::Variant value);

    void process(std::string input);
    void emitAdd(std::string collectionName, std::string id, jvar::Variant fields);
    void emitChange(std::string collectionName, std::string id, jvar::Variant fields);
    void emitRemove(std::string collectionName, std::string id);

    void registerMethod(std::string name, jvar::Variant (*method)(jvar::Variant &, jvar::Variant &));

};


#endif // DDPSERVER_H
