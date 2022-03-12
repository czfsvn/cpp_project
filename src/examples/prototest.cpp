#include "addressbook.pb.h"
#include <iostream>

using namespace std;

namespace test_proto
{
    void main()
    {
        msg::AddressBook addr;
        std::cout << "test proto\n";
    }
}  // namespace test_proto

namespace ns_proto
{
    void main()
    {
        test_proto::main();
    }
}  // namespace ns_proto