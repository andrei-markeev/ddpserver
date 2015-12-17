### DDP Server for C++

This C++ library implements server endpoint for Meteor's [DDP protocol v1](https://github.com/meteor/meteor/blob/devel/packages/ddp/DDP.md).

### Use cases

- Provide a custom live data source from your C++ application to Meteor frontend
- Call your C++ methods from Meteor

### Dependencies

1. [jvar](https://github.com/YasserAsmi/jvar) - low-memory JSON serialization library

### Build & install

1. wget https://github.com/andrei-markeev/ddpserver/archive/master.tar.gz
2. tar zxf master.tar.gz
3. cd ddpserver-master
4. mkdir build
5. cd build
6. cmake ..
7. make
8. make install

### Use

DdpServer only processes DDP protocol. It should be used with a websocket
library such as [websocketpp](https://github.com/zaphoyd/websocketpp).

C++:

```cpp
#include <jvar/jvar.h>
#include <ddpserver.h>

DdpServer ddpServer;

jvar::Variant myMethod1(jvar::Variant& env, jvar::Variant& args)
{
	if (args.length() < 2)
		throw;

	return args[0] + args[1];
}

void onEmit(jvar::Variant& env, std::string s)
{
	// use your favorite websocket library and send s to websocket
}

void websocket_on_message(std::string message)
{
	ddpServer.process(message);
}

int main(int argc, char** argv)
{

	// ...

	ddpServer(onEmit);
	ddpServer.registerMethod("myMethod1", myMethod1);

	jvar::Variant data;
	data.createObject();
	data["something"] = "value";

	ddpServer.emitAdd("coll1", "element1_id", data);


	data["something"] = "new value";
	ddpServer.emitChange("coll1", "element1_id", data);

	ddpServer.emitRemove("coll1", "element1_id");
	
	// ...
	
	
}

```

JavaScript (client-side):

```javascript
MyConnection = DDP.connect("http://localhost:9002");

Collection1 = new Mongo.Collection("coll1", { connection: MyConnection });

MyConnection.call("myMethod1", "param1", "param2", (error, result) => {
	if (error)
		alert(error.reason);
	else
		console.log(result);
});


```

### Roadmap

 - Subscriptions
 - Example of integrating with websocketpp
 - Example of Meteor authentication
