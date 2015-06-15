#include <handy/conn.h>
#include <handy/logging.h>
#include <handy/daemon.h>
#include <handy/codec.h>

using namespace std;
using namespace handy;

void reconnect2(EventBase* base, string host, short port) {
    TcpConnPtr con = TcpConn::createConnection(base, host, port);
    con->setCodec(new LengthCodec);
    con->onState([=](const TcpConnPtr& con) {
        info("onState called state: %d", con->getState());
        if (con->getState() == TcpConn::Connected) {
            con->sendMsg("hello");
        } else if (con->getState() == TcpConn::Closed || con->getState() == TcpConn::Failed) {
            base->runAfter(3000, [=] { reconnect2(base, host, port); });
        }
    });
    con->onMsg([](const TcpConnPtr& con, Slice msg) {
        info("recv msg: %.*s", (int)msg.size(), msg.data());
    });
}

int main(int argc, const char* argv[]) {
    setloglevel("TRACE");
    EventBase base;
    Signal::signal(SIGINT, [&]{ base.exit(); });
    reconnect2(&base, "localhost", 99);
    base.loop();
    info("program exited");
}