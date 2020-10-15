#include <iostream>

#include <boost/asio.hpp>

#include <boost/bind.hpp>

//#include<boost/asio/serial_port.hpp>

using namespace std;

using namespace boost::asio;

 

int main(int argc, char* argv[])

{
    io_service iosev;
    
    cout<< "start to set port No" << endl;

    serial_port sp(iosev,"/dev/pts/3");

    cout << "port created "<< endl;
    //设置参数
    sp.set_option(serial_port::baud_rate(115200));
    sp.set_option(serial_port::flow_control(serial_port::flow_control::none));
    sp.set_option(serial_port::parity(serial_port::parity::none));
    sp.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
    sp.set_option(serial_port::character_size(8));

    cout<< "port parameter setup" << endl;

    // 向串口写数据
    string msg;
    std::cin>>msg;
    write(sp, buffer(msg));

    // 向串口读数据
    char buf[10000];
    read(sp, buffer(buf));
    cout<< buf << endl;

    iosev.run();
    return 0;

}
