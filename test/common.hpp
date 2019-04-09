#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <cassert>

#define TEST_CLASS(NAME)    namespace CLASS##NAME {
#define TEST_CLASS_END()    }
#define TEST_METHOD(NAME)   static int METHOD##NAME = [] {
#define TEST_METHOD_END()   return 0; }();


struct Assert
{
template <typename T1, typename T2>
static void AreEqual(const T1 &a, const T2 &b)
{
    assert(a == b);
}
};


int main(int argc, char *agrv[])
{
    return 0;
}


#endif // TEST_COMMON_HPP
