### DDP Server for C++

This C++ library implements server endpoint for Meteor's [DDP protocol v1](https://github.com/meteor/meteor/blob/devel/packages/ddp/DDP.md).

### Use cases

- Provide a custom live data source from your C++ application to Meteor frontend
- Call your C++ methods from Meteor

### Dependencies

  * CMake 2.8+

  * Modern GCC/Clang

### Build & install

  * mkdir build

  * cd build

  * cmake ..

  * make

  * make install

### Use

DdpServer only processes DDP protocol itself. It should be used with some websocket library. Examples of integrating with following libraries are available:
 - [mongoose](https://github.com/cesanta/mongoose) - [examples/mongoose.cpp](https://github.com/andrei-markeev/ddpserver/blob/master/examples/mongoose.cpp).
 - [websocketpp](https://github.com/zaphoyd/websocketpp) - [examples/websocketpp.cpp](https://github.com/andrei-markeev/ddpserver/blob/master/examples/websocketpp.cpp).
 - [libwebsockets](https://libwebsockets.org) - [examples/libwebsockets.cpp](https://github.com/andrei-markeev/ddpserver/blob/master/examples/libwebsockets.cpp).

Other libraries can be integrated similarly.

Example below uses websocketpp.

C++:

```cpp

jvar::Variant myMethod1(void *context, jvar::Variant& args)
{
	return args[0] + args[1];
}

void emitCallback(void *context, const std::string &output)
{
    websocketpp::connection_hdl hdl = *(websocketpp::connection_hdl *)context;
    wsServer->send(hdl, output.c_str(), output.length(), websocketpp::frame::opcode::text);
}

void websocket_on_message(websocketpp::connection_hdl hdl, websocket_server::message_ptr msg)
{
    ddpServer->setContext(&hdl);
    ddpServer->process(msg->get_payload());
}

// .. skipped ..

int main(int argc, char** argv)
{
	ddpServer = new DdpServer(emitCallback);
	ddpServer->registerMethod("myMethod1", myMethod1);

	// .. skipped ..
}

```

JavaScript (client-side):

```javascript
MyConnection = DDP.connect("http://localhost:9002");

MyConnection.call("myMethod1", "param1", "param2", (error, result) => {
	if (error)
		alert(error.reason);
	else
		console.log(result);
});

```

Result:

![screenshot](https://github.com/andrei-markeev/ddpserver/blob/master/examples/websocketpp.png)

### Live data

It is also possible to implement Meteor live data:

C++:

```cpp
jvar::Variant data;
data.createObject();
data.addProperty("banana", "value");
data.addProperty("tomato", 1234);
data.addProperty("kiwi", "something");
ddpServer->emitAdd("coll1", "element1_id", data);

// ...

jvar::Variant changeData;
changeData.createObject();
changeData.addProperty("banana", "new_value");
ddpServer->emitChange("coll1", "element1_id", changeData);

// ...

ddpServer->emitRemove("coll1", "element1_id");	
```

JS:

```js
Collection1 = new Mongo.Collection("coll1", { connection: MyConnection });
```

Subscriptions aren't supported for now, all emits will be immediately published
to the frontend. Subscriptions are planned, but for now you can easily emulate
them using methods.

### Project roadmap

 - Example of Meteor authentication
 - Subscriptions support

