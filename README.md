  
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

For this library you can write your own low-level transport using a_client and a_server as absractions(btw, here is my <a href = "https://github.com/enoreex/win_sockets">implementation</a> for windows). Maybe someday I will create several such solutions for different platforms.

<h2 tabindex="-1" dir="auto"><a class="anchor" aria-hidden="true"></a>Usage</h2>

<h3>Client:</h3>

```cpp
void start()
{
	rudp_client client;
	client.connected_callback = [this, &client]()
	{
		ice_data::write data;

		data.add_string("Hello World!");

		client.send_reliable(data);
	};

	client.connect<win_udp_client>(end_point("127.0.0.1", 7777), end_point(0, 0));

	std::thread tick_t([&]() { while (true) client.update(); });
	tick_t.join();
}
```

<h3>Server:</h3>

```cpp
void start()
{
	rudp_server server;
	server.try_start<win_udp_server>(end_point(0, 7777));
	std::thread tick_t([&]() { while (true) server.update(); });

	server.external_data_callback = [this](rudp_connection& c, ice_data::read& d) 
	{
		handle(c, d); 
	};

	tick_t.join();
}

void handle(rudp_connection& c, ice_data::read& data)
{
	std::cout << c.get_remote_point().get_port_str() << ": " << data.get_string() << "\n";
}
```

