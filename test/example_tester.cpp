#include "common.hpp"


// Simple class for example
class Example
{
  public:
    static int add(int a, int b)
    {
        return a + b;
    }
};


TEST_CLASS(Example)

    TEST_METHOD(add)

        Assert::AreEqual(3 + 4, Example::add(3, 4));

    TEST_METHOD_END()

TEST_CLASS_END()
