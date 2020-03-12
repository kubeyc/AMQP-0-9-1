#include <iostream>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char *argv[])
{
    int AMQPServerSockFD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in AMQPServerAddress;
    memset(&AMQPServerAddress, 0, sizeof(struct sockaddr_in));
    AMQPServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, "localhost", &AMQPServerAddress.sin_addr.s_addr);
    AMQPServerAddress.sin_port = htons(5672);

    int ret = connect(AMQPServerSockFD, (struct sockaddr*)&AMQPServerAddress, sizeof(struct sockaddr_in));
    if (ret == -1) {
        std::cout << "connect error" << std::endl;
        return 0;
    }
    std::cout << "connect successful" << std::endl;

    // send protocol header
    char headerbuf[8] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    send(AMQPServerSockFD, headerbuf, 8, 0);




    /*
     general frame

       0      1         3             7                  size+7 size+8
      +------+---------+-------------+  +------------+  +-----------+
      | type | channel |     size    |  |  payload   |  | frame-end |
      +------+---------+-------------+  +------------+  +-----------+
       octet   short         long         size octets       octet

     */
    char buf[7];
    ret = recv(AMQPServerSockFD, buf, 7, 0);
    std::cout << "recv ret: " << ret << std::endl;

    int type, channel, size;
    type = buf[0];
    channel = buf[2] & 0x00ff | (buf[1] << 8) & 0xff00;
    size =  buf[6] & 0x000000ff
            | (buf[5] << 8) & 0x0000ff00
            | (buf[4] << 16) & 0x00ff0000
            | ((buf[3]) << 24) & 0xff000000;
    std::cout << "type: " << type << ", channel: "  << channel << ", size: " << size <<std::endl;

    // type =1, method payload
    char payloadbuf[size];

    ret = recv(AMQPServerSockFD, payloadbuf, size, 0);

    std::cout << "recv ret: " << ret << std::endl;

    int classid, methodid;
    classid = payloadbuf[1] & 0x00ff | (payloadbuf[0] << 8) & 0xff00;
    methodid = payloadbuf[3] & 0x00ff | (payloadbuf[0] << 8) & 0xff00;
    std::cout << "class-id: " << classid << ", method-id: " << methodid << std::endl;

    // class-id = 10, connection. method-id = 10, connect->start
    int versionmajor, versionminor;
    versionmajor = payloadbuf[4];
    versionminor = payloadbuf[5];
    std::cout << "version-major: " << versionmajor << ", version-minor: " << versionminor << std::endl;

    // todo parse server properties

    // todo parse frame-end
}
