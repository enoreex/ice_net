  
<p align="center">
  <img src="https://github.com/enoreex/ICENet/assets/125078218/c28309e2-377a-450f-9440-8b7e8eaf335a" alt="logo" width="200" height="200">
</p>

<h1 align="center" tabindex="-1" dir="auto"><a class="anchor" aria-hidden="true"></a>ICENet C++</h1>

My convoluted implementation of a reliable protocol. If this is to be used, it should be used at most in a local network. This implementation is similar to my existing <a href = "https://github.com/enoreex/ICENet/">C# lib</a>

<h2 tabindex="-1" dir="auto"><a class="anchor" aria-hidden="true"></a>My future additions</h2>

At the moment, the library implements a fairly convenient and clear system of package deserialization and deserialization. It has a convenient logging system.

<h3>The library now has:</h3>

<ul>
  <li style="font-size: smaller;">Installation and support connections (RUDP functionality)</li>
  <li style="font-size: smaller;">Handle/Send reliable packets (RUDP functionality)</li>
  <li style="font-size: smaller;">Convenient logging system</li>
  <li style="font-size: smaller;">Minimum thread safety system</li>
  <li style="font-size: smaller;">Ability to change the low-level transport</li>
</ul>

For this library you can write your own low-level transport using a_client and a_server as absractions(Btw, I use win-sockets for the example, they come with the library). Maybe someday I will create several such solutions for different platforms.

<h3>CMake (.lib) or (.dll): </h3>

Included are folders with an ending

<ul>
  <li style="font-size: smaller;>If you are using C++ then i recomend .lib</li>
  <li style="font-size: smaller;>If you are using C#(P/Invoke) for example, then i recommend .dll</li>
</ul>

.lib:
<ul>
  <li style="font-size: smaller;></li>
</ul>

<h2 tabindex="-1" dir="auto"><a class="anchor" aria-hidden="true"></a>Simplest example</h2>

<h3>Client:</h3>

```cpp
void start()
{
	rudp_client client; //creating client
	client.socket = new win_udp_client; //winsock as transport

	client.connected_callback = [this, &client]() //will be executed when client connected
	{
		ice_data::write data;

		data.add_string("Hello World!");

		client.send_reliable(data);
	};

	client.connect(end_point("127.0.0.1", 7777), end_point(0, 0)); //connecting... (0, 0) - means auto ip + port

	std::thread tick_t([&]() { while (true) client.update(); }); //will be bad without update(). more often is better.
	tick_t.join();
}
```

<h3>Server:</h3>

```cpp
void start()
{
	rudp_server server; //creating server
	server.socket = new win_udp_server; //winsock as transport

	server.external_data_callback = [this](rudp_connection& c, ice_data::read& d) //will be executed when packet handled
	{
		handle(c, d); 
	};

	server.try_start(end_point(0, 7777)); //starting... (0, 7777) - means auto ip + 7777

	std::thread tick_t([&]() { while (true) server.update(); }); //will be bad without update(). more often is better.
	tick_t.join();
}

void handle(rudp_connection& c, ice_data::read& data)
{
	std::cout << c.get_remote_point().get_port_str() << ": " << data.get_string() << "\n";
}
```

