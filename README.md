Open Netcode
======

Introduction
------
Multiplayer games are and always will be very hard to develop, having in mind that out-of-the-box solutions are highly unoptimized and hard to configure, while developing your own netcode from scratch can be a project by itself.

Having that in mind, it's a matter of firing up the server, then how many clients you wish.

Objective
------
The objective of this project is to develop a solid foundation for a [valve/quake networking model](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking) without over-generalization, that developers can use to bootstrap their multiplayer game.

**It's not meant to be a plug-in where you check a few boxes and your game is now networked.** The objective of this project is providing a starting point to estabilish the code base, this approach helps the code itself be easy to understand and extremely resource efficient (network bandwidth, CPU and latency).

Folder Structure and Files
------
We're including in this repository a range of netcode models, each netcode model will have two folders, the server and the client. We're starting with a first person shooter model, so we have now two folders. Inside each folder is a common Unreal Engine 4 Project.
- FirstPersonClient: First person shooter model client
- FirstPersonServer: First person shooter model server
- run_2_clients.bat: Example bat script on how to fire up two client windows without having to open multiple engines.
- run_sandboxied_client.bat: Example bat script on how to fire up one client window without having to open multiple engines.

Networking  Top-Level Specifications
------
- Server: Authoritative
- Protocol: UDP
- Server Tick Rate: Fixed Configurable
- Client Tick Rate: Variable
- Sockets Library: [Unreal Multi-Platform Socket Subsystem](https://api.unrealengine.com/INT/API/Runtime/Sockets/ISocketSubsystem/index.html)
- Sockets Implementation: Threaded with blocking calls.
- Physics: Navmesh based.
