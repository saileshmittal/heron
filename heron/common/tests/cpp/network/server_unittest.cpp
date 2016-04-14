#include <iostream>

#include "basics/basics.h"
#include "errors/errors.h"
#include "threads/threads.h"
#include "network/network.h"

#include "network/unittests.pb.h"
#include "network/server_unittest.h"

TestServer::TestServer
 (
   EventLoopImpl*         eventLoop,
   const NetworkOptions& _options
 ) : Server(eventLoop, _options)
{
  InstallMessageHandler(&TestServer::HandleTestMessage);
  InstallMessageHandler(&TestServer::HandleTerminateMessage);
  nrecv_ = nsent_ = 0;
}

TestServer::~TestServer()
{
}

void
TestServer::HandleNewConnection(Connection* _conn)
{
  if (clients_.find(_conn) != clients_.end())
    return;

  clients_.insert(_conn);
  vclients_.push_back(_conn);
}

void
TestServer::HandleConnectionClose
 (
   Connection*      _conn,
   NetworkErrorCode _status __attribute__((unused))
 )
{
  if (clients_.find(_conn) == clients_.end())
    return;

  clients_.erase(_conn);

  std::vector<Connection*>::iterator it =
    std::remove(vclients_.begin(), vclients_.end(), _conn);

  vclients_.erase(it, vclients_.end());
}

void
TestServer::HandleTestMessage
 (
   Connection*  _connection __attribute__((unused)),
   TestMessage* _message
 )
{
  nrecv_++;

  // find a random client to send the message to
  sp_int32 r = rand() % vclients_.size();

  Connection* sc = vclients_[r];
  SendMessage(sc, *_message);

  nsent_++;
}

void TestServer::Terminate()
{
  Stop();
  getEventLoop()->loopExit();
}

void
TestServer::HandleTerminateMessage
 (
   Connection*         _connection __attribute__((unused)),
   TerminateMessage*   _message __attribute__((unused))
 )
{
  AddTimer([this] () { this->Terminate(); }, 1);
}