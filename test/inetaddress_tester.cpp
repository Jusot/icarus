#include "../icarus/inetaddress.hpp"
#include <cassert>

using namespace std;
using namespace icarus;

int main()
{
    InetAddress addr1;
    assert(addr1.to_ip_port() == "0.0.0.0:0");

    InetAddress addr2(5656, true);
    assert(addr2.to_ip_port() == "127.0.0.1:5656");

    InetAddress addr3("192.168.24.1", 66);
    assert(addr3.to_ip_port() == "192.168.24.1:66");
    assert(addr3.to_ip() == "192.168.24.1");
    assert(addr3.to_port() == 66);
}