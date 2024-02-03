  
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
  <li style="font-size: smaller;">If you are using C++ then i recomend .lib</li>
  <li style="font-size: smaller;">If you are using C#(P/Invoke) for example, then i recommend .dll</li>
</ul>

```

